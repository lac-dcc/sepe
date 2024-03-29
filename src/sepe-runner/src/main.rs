use std::{fs, io::Write, os::unix::ffi::OsStringExt, str::FromStr};

use clap::{Parser, ValueEnum};
use toml::Table;

const TOML_FILENAME: &str = "Regexes.toml";
const KEYGEN: &str = "keygen";
const KEYUSER: &str = "keyuser";
const KEYUSER_DEBUG: &str = "keyuser-debug";
const KEYBUILDER: &str = "keybuilder";

#[derive(Debug, Clone, Copy, ValueEnum)]
/// Distribution to use when randomly generating the characters
enum Distribution {
    /// Uniform distribution
    Uniform,
    /// Normal distribution
    Normal,
    /// Incremental distribution (VERY SLOW)
    ///
    /// For example, a regex like [0-9]{3} will produce '001', '002', '003', and so on, in order.
    Incremental,
}

impl Distribution {
    fn as_str(&self) -> &'static str {
        match self {
            Distribution::Uniform => "uniform",
            Distribution::Normal => "normal",
            Distribution::Incremental => "incremental",
        }
    }
}

/// Helper program that stitches together the other programs in this repository
///
/// It is meant to facilitate testing and benchmarking during development and research.
///
/// **End users who only want to synthesize some functions do not have to interact with this**.
/// They should just use keybuilder and keysynth directly, instead
#[derive(Parser, Debug)]
#[command(author, version, name = "sepe-runner")]
struct Command {
    /// Number of keys to generate
    ///
    /// This is set to 10000 when `--histogram` is set or if distribution is 'incremental'
    #[clap(
        short,
        long,
        default_value = "1000000",
        default_value_if("histogram", "true", "10000"),
        default_value_if("distribution", "incremental", "10000")
    )]
    keys: u64,

    /// Distribution used in random generation
    #[clap(short, long, default_value = "uniform")]
    distribution: Distribution,

    /// Key generator random number generator seed
    #[clap(long, default_value = "223554")]
    keygen_seed: u64,

    /// Whether to run keyuser in debug mode
    #[clap(long, default_value = "false")]
    debug: bool,

    /// Key user random number generator seed
    #[clap(long, default_value = "9764096")]
    keyuser_seed: u64,

    /// Number of repetitions to forward to keyuser
    #[clap(short, long, default_value = "1")]
    repetitions: u64,

    /// Number of operations to run
    #[clap(short, long, default_value = "10000")]
    operations: u64,

    /// Percentage of insertion operations. **Must be in [0; 1] range**
    #[clap(short, long, default_value = "0.5")]
    insert: f64,

    /// Percentage of search operations. **Must be in [0; 1] range**
    #[clap(short, long, default_value = "0.3")]
    search: f64,

    /// Percentage of elimination operations. **Must be in [0; 1] range**
    #[clap(short, long, default_value = "0.2")]
    elimination: f64,

    /// Prints configuration and runs keyuser in verbose mode
    #[clap(short, long, default_value = "false")]
    verbose: bool,

    /// Suffix of output csv files.
    ///
    /// The prefix is the Regex's entry name in the Regexes.toml file.
    /// When `--histogram` is set, the default value is '_distribution.py'
    #[clap(
        long,
        default_value = "_performance.csv",
        default_value_if("histogram", "true", "_distribution.py")
    )]
    outfile: String,

    /// Generate the synthesized function for the given regex, do not run experiments
    #[clap(long)]
    synthesize: bool,

    /// Generate the distribution histogram for the given regex, do not run experiments
    #[clap(long)]
    histogram: bool,

    /// Measure the performance of the hash functions without containers
    #[clap(long)]
    hash_performance: bool,

    /// Whether to generate the keys incrementally, rather than randomly (VERY SLOW)
    #[clap(long)]
    incremental_generation: bool,

    /// Regexes we will benchmark, defined in Regexes.toml
    ///
    /// You can send multiple Regexes, separated by spaces
    regexes: Vec<String>,
}

fn find_file(filename: &str) -> fs::DirEntry {
    let mut cwd = std::env::current_exe().expect("couldn't find out current executable path");
    cwd.pop();
    loop {
        let entries = cwd
            .read_dir()
            .unwrap_or_else(|e| panic!("failed to read directory {:#?}, {e}", cwd));

        match entries
            .flatten()
            .find(|entry| entry.file_name().eq(filename))
        {
            Some(entry) => return entry,
            None => {
                cwd = cwd
                    .parent()
                    .unwrap_or_else(|| panic!("failed to find {} file", filename))
                    .into()
            }
        }
    }
}

fn read_toml_file() -> Table {
    let entry = find_file(TOML_FILENAME);
    let file_bytes = fs::read(entry.path())
        .unwrap_or_else(|e| panic!("failed to read {:#?} file: {e}", entry.file_name()));
    let file_content = std::str::from_utf8(&file_bytes)
        .unwrap_or_else(|e| panic!("failed to decode {:#?} file's utf8: {e}", entry.file_name()));

    Table::from_str(file_content).expect("failed to parse toml file's content")
}

fn main() {
    let mut cmd = Command::parse();

    let regex_table = read_toml_file();

    if cmd.regexes.contains(&"ALL".to_string()) {
        cmd.regexes.clear();
        cmd.regexes = regex_table.keys().map(|key| key.to_string()).collect();
    }

    for cmd_regex in cmd.regexes {
        let values = match regex_table.get(&cmd_regex) {
            Some(r) => r,
            None => {
                eprintln!("there is {} entry in {}", cmd_regex, TOML_FILENAME);
                continue;
            }
        };

        let regex = values
            .get("regex")
            .unwrap_or_else(|| panic!("{} entry has no 'regex' value!", cmd_regex))
            .as_str()
            .unwrap_or_else(|| panic!("{} regex entry must be a string!", cmd_regex));

        let hashes = values
            .get("hashes")
            .unwrap_or_else(|| panic!("{} entry has no 'hashes' list!", cmd_regex))
            .as_array()
            .unwrap_or_else(|| panic!("{} hashes entry must be a list of strings!", cmd_regex));

        let keygen = find_file(KEYGEN);
        let keyuser = find_file(if cmd.debug { KEYUSER_DEBUG } else { KEYUSER });

        use std::process::Command as Cmd;

        let keygen_cmd = Cmd::new(keygen.path())
            .stdout(std::process::Stdio::piped())
            .arg(regex)
            .arg("-n")
            .arg(format!("{}", cmd.keys))
            .arg("-s")
            .arg(format!("{}", cmd.keygen_seed))
            .arg("-d")
            .arg(cmd.distribution.as_str())
            .spawn()
            .expect("failed to spawn keygen command");

        let keygen_out = keygen_cmd.stdout.expect("failed to open keygen stdout");

        if cmd.synthesize {
            let mut keybuilder_output = Cmd::new(find_file(KEYBUILDER).path())
                .stdin(std::process::Stdio::from(keygen_out))
                .output()
                .expect("failed to spawn keybuilder!");

            keybuilder_output.stdout.pop();

            let args = std::ffi::OsString::from_vec(keybuilder_output.stdout);
            let args = args.to_string_lossy();
            let args: Box<[String]> = args.split_whitespace().map(|e| e.to_string()).collect();
            match Cmd::new(find_file("keysynth").path())
                .args(args.iter())
                .spawn()
                .expect("failed to spawn keysynth!")
                .wait()
            {
                Ok(exit_status) => {
                    if !exit_status.success() {
                        eprintln!("ERROR: keysynth failed!");
                    }
                }
                Err(e) => {
                    eprintln!("ERROR: keysynth couldn't run to completion: {e}!");
                }
            }

            continue;
        }

        let mut keyuser_cmd = Cmd::new(keyuser.path());

        keyuser_cmd
            .stdin(std::process::Stdio::from(keygen_out))
            .arg("-i")
            .arg(format!("{}", (cmd.insert * 100.0) as u64))
            .arg("-s")
            .arg(format!("{}", (cmd.search * 100.0) as u64))
            .arg("-e")
            .arg(format!("{}", (cmd.elimination * 100.0) as u64))
            .arg("-n")
            .arg(format!("{}", cmd.operations))
            .arg("-seed")
            .arg(format!("{}", cmd.keyuser_seed))
            .arg("-r")
            .arg(format!("{}", cmd.repetitions))
            .arg("--distribution")
            .arg(cmd.distribution.as_str());

        if cmd.verbose {
            keyuser_cmd.arg("--verbose");
        }

        if cmd.histogram {
            keyuser_cmd.arg("--test-distribution");
        }

        if cmd.hash_performance {
            keyuser_cmd.arg("--hash-performance");
        }

        keyuser_cmd.arg("--hashes");
        for hash in hashes {
            keyuser_cmd.arg(hash.as_str().unwrap_or_else(|| {
                panic!("{} hashes entry must be a list of strings!", cmd_regex)
            }));
        }

        println!("\nExecuting {} regex: {}", cmd_regex, regex);
        if cmd.verbose {
            println!("    Configuration:");
            println!("        Debug: {}", cmd.debug);
            println!("        Keys Generated:       {}", cmd.keys);
            println!("        Number of Operations: {}", cmd.operations);
            println!(
                "        [Insertion, Search, Elimination ] Percentages: [{}, {}, {}]",
                cmd.insert, cmd.search, cmd.elimination
            );
        }

        let keyuser_out = keyuser_cmd
            .output()
            .expect("failed to spawn keyuser command");

        if !keyuser_out.status.success() {
            eprintln!("        !!!FAILED: {}!!!", keyuser_out.status);
        } else {
            let filename = cmd_regex + &cmd.outfile;
            let mut outfile =
                std::fs::File::create(filename).expect("failed to create output file!");
            outfile.write_all(&keyuser_out.stdout).unwrap();
        }
        std::io::stderr().write_all(&keyuser_out.stderr).unwrap();
    }
}
