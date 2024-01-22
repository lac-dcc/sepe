use std::{fs, io::Write, path::PathBuf, str::FromStr};

use clap::Parser;
use toml::Table;

const TOML_FILENAME: &str = "Regexes.toml";
const KEYGEN: &str = "keygen";
const KEYUSER: &str = "keyuser";
const KEYUSER_DEBUG: &str = "keyuser-debug";

#[derive(Parser, Debug)]
#[command(version, name = "bench-runner")]
struct Command {
    /// Number of keys to generate
    #[clap(short, long, default_value = "1000000")]
    keys: u64,

    /// Key generator random number generator seed
    #[clap(long, default_value = "223554")]
    keygen_seed: u64,

    /// Whether to run keyuser in debug mode
    #[clap(short, long, default_value = "false")]
    debug: bool,

    /// Key user random number generator seed
    #[clap(long, default_value = "9764096")]
    keyuser_seed: u64,

    /// Number of operations to run
    #[clap(short, long, default_value = "1000000")]
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

    /// Suffix of output files
    #[clap(long, default_value = "_results.csv")]
    outfile: String,

    /// regexes we will benchmark
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
            .spawn()
            .expect("failed to spawn keygen command");

        let keygen_out = keygen_cmd.stdout.expect("failed to open keygen stdout");

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
            .arg(format!("{}", cmd.keyuser_seed));

        if cmd.verbose {
            keyuser_cmd.arg("--verbose");
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
            eprintln!("        !!!FAILED!!!");
        }

        let mut outfile =
            std::fs::File::create(cmd_regex + &cmd.outfile).expect("failed to create output file!");
        outfile.write_all(&keyuser_out.stdout).unwrap();
        std::io::stdout().write_all(&keyuser_out.stdout).unwrap();
        std::io::stderr().write_all(&keyuser_out.stderr).unwrap();
    }
}
