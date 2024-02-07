use std::{
    io::{stdout, BufWriter, Write},
    num::NonZeroU32,
};

use clap::{Parser, ValueEnum};
use rand::{Rng, SeedableRng};
use rand_distr::{Distribution as Dist, Normal, Uniform};

/// Represents a List (e.g. `[0-9]`) of possible values
#[derive(Debug)]
struct List {
    inner: String,
    /// A complement is a list with a caret: [^0-9]. It means we should generate all values BUT
    /// those in the list
    complement: bool,
}

impl List {
    /// Returns a random character from the list
    pub fn generate<R: Rng, D: Dist<f64>>(&self, r: &mut R, d: &D) -> char {
        if !self.complement {
            self.inner
                .chars()
                .nth((d.sample(r).rem_euclid(1.0) * (self.inner.len() - 1) as f64) as usize)
                .unwrap()
        } else {
            let mut ch = rand::random();
            while self.inner.contains(ch) {
                ch = (ch as u8 + 1) as char;
            }
            ch
        }
    }

    /// Returns the specified character from the list, decrementing the `i` accordingly
    pub fn generate_inc(&self, i: &mut u64) -> char {
        if !self.complement {
            let ch = self.inner.chars().cycle().nth(*i as usize).unwrap();
            let n = self.inner.chars().count() as u64;
            *i = if *i < n { 0 } else { *i / n };
            ch
        } else {
            let mut ch =
                char::from_u32(*i as u32 + 32).expect("failed to generate incremental '.'");
            while self.inner.contains(ch) {
                ch = (ch as u8 + 1) as char;
            }
            *i = (*i).saturating_sub(1);
            ch
        }
    }
}

/// Possible Regex symbols
#[derive(Debug)]
enum RegexSymbol {
    /// stands for the exact character it contains
    Literal(char),
    /// a parenthesized group, e.g. `(inner_regex)`
    Group(Vec<Regex>),
    /// list, e.g. `[0-9]`
    List(List),
}

/// A Regex is a symbol and its possible modifiers
///
/// In this program, we only accept repetitions `{n}` as modifiers
/// The operators `+`, `*` and `?` are all not accepted
#[derive(Debug)]
struct Regex {
    symbol: RegexSymbol,
    repetitions: Option<NonZeroU32>,
}

impl Regex {
    /// Initializes a new Regex from a symbol, setting all modifiers to false
    pub fn new(symbol: RegexSymbol) -> Self {
        Self {
            symbol,
            repetitions: None,
        }
    }

    /// Returns random characters according to the regular expression rules
    pub fn generate<R: Rng, D: Dist<f64>>(&self, r: &mut R, d: &D) -> String {
        let repetitions = self.repetitions.map(|r| r.get()).unwrap_or(1);
        let mut s = String::with_capacity(repetitions as usize);
        for _ in 0..repetitions {
            match &self.symbol {
                RegexSymbol::Literal(literal) => s.push(*literal),
                RegexSymbol::Group(group) => s.push_str(&generate(group, r, d)),
                RegexSymbol::List(list) => s.push(list.generate(r, d)),
            }
        }

        s
    }

    /// Returns incremental characters according to the regular expression rules
    pub fn generate_inc(&self, i: &mut u64) -> String {
        let repetitions = self.repetitions.map(|r| r.get()).unwrap_or(1);
        let mut s = String::with_capacity(repetitions as usize);
        for _ in 0..repetitions {
            match &self.symbol {
                RegexSymbol::Literal(literal) => s.insert(0, *literal),
                RegexSymbol::Group(group) => s = generate_inc(group, i) + &s,
                RegexSymbol::List(list) => s.insert(0, list.generate_inc(i)),
            }
        }

        s
    }
}

/// generate a random regex
fn generate<R: Rng, D: Dist<f64>>(regexes: &[Regex], r: &mut R, d: &D) -> String {
    let mut s = String::with_capacity(regexes.len());
    for regex in regexes {
        s.push_str(&regex.generate(r, d));
    }
    s
}

/// generate a specific regex, according to `i`
fn generate_inc(regexes: &[Regex], i: &mut u64) -> String {
    let mut s = String::with_capacity(regexes.len());
    for regex in regexes.iter().rev() {
        s = regex.generate_inc(i) + &s;
    }
    s
}

/// parse a list ([0-9])
fn parse_list(chars: &mut std::str::Chars) -> Regex {
    let mut inner = String::new();

    let first = chars.next().unwrap();
    let complement = first == '^';
    if !complement {
        inner.push(first);
    }

    while let Some(ch) = chars.next() {
        if ch == ']' {
            break;
        } else if ch == '-' {
            let prev = match inner.pop() {
                Some(ch) => ch,
                None => {
                    inner.push('-');
                    continue;
                }
            };

            let next = chars.next().unwrap();
            if next == ']' {
                inner.push(prev);
                inner.push('-');
                break;
            }

            for ch in prev..=next {
                inner.push(ch);
            }
        } else {
            inner.push(ch);
        }
    }

    Regex::new(RegexSymbol::List(List { inner, complement }))
}

/// parse a list ([0-9])
fn parse_repetitions(chars: &mut std::str::Chars) -> NonZeroU32 {
    let mut s = String::new();
    for ch in chars.by_ref() {
        if ch == '}' {
            break;
        }
        s.push(ch);
    }

    let uint: u32 = s
        .trim()
        .parse()
        .unwrap_or_else(|_| panic!("failed to parse: {} as an unsigned integer", s));
    NonZeroU32::new(uint).expect("integer in a '{}' repetition must be > 0")
}

/// parse a group ((inner_regex))
fn parse_group(chars: &mut std::str::Chars) -> Regex {
    use RegexSymbol::*;

    let mut group: Vec<Regex> = Vec::new();

    while let Some(ch) = chars.next() {
        match ch {
            '\\' => {
                let next = chars.next().unwrap();
                group.push(Regex::new(Literal(next)));
            }
            '[' => group.push(parse_list(chars)),
            '{' => group.last_mut().unwrap().repetitions = Some(parse_repetitions(chars)),
            ')' => break,
            '(' => group.push(parse_group(chars)),
            ch => group.push(Regex::new(Literal(ch))),
        }
    }

    Regex::new(Group(group))
}

/// parse the regex
fn parse_regex(mut chars: std::str::Chars) -> Vec<Regex> {
    use RegexSymbol::*;

    let mut tree: Vec<Regex> = Vec::new();

    while let Some(ch) = chars.next() {
        match ch {
            '\\' => {
                let next = chars.next().unwrap();
                tree.push(Regex::new(Literal(next)));
            }
            '[' => tree.push(parse_list(&mut chars)),
            '{' => tree.last_mut().unwrap().repetitions = Some(parse_repetitions(&mut chars)),
            ')' => panic!("badly formatted regex!"),
            '(' => tree.push(parse_group(&mut chars)),
            ch => tree.push(Regex::new(Literal(ch))),
        }
    }

    tree
}

#[derive(Clone, Copy, ValueEnum)]
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

#[derive(Parser)]
#[command(version, name = "keygen")]
/// `keygen` generates random strings based on a regex
///
/// Note **DO NOT IMPLEMENT** several regexes operations:
///   * the `*` operator
///   * the `+` operator
///   * the `?` operator
///   * the `.` operator
///
/// Regexes passed to this program may only make use of `()` groups, `[]` lists of characters and
/// `{n}`, where `n` is an integer. For example, to generate 3 digit numbers, you would use:
/// `[0-9]{3}`
struct Command {
    /// Regex used to generate the strings
    ///
    /// Attention! The OR operator (|) is not implemented!
    regex: String,

    /// Number of elements to generate
    #[clap(short, long, default_value = "100")]
    number: u64,

    /// Seed used for random number generation
    #[clap(short, long, default_value = "223554")]
    seed: u64,

    /// Distribution used in random generation
    #[clap(short, long, default_value = "uniform")]
    distribution: Distribution,
}

fn main() {
    let cmd = Command::parse();

    let mut r = rand::rngs::StdRng::seed_from_u64(cmd.seed);

    let regex = parse_regex(cmd.regex.chars());

    #[cfg(debug_assertions)]
    {
        println!("{regex:#?}");
    }

    let stdout = stdout();
    let lock = stdout.lock();
    let mut writer = BufWriter::new(lock);
    match cmd.distribution {
        Distribution::Uniform => {
            let d = Uniform::new(0.0, 1.0);
            for _ in 0..cmd.number {
                writeln!(writer, "{}", generate(&regex, &mut r, &d)).unwrap();
            }
        }
        Distribution::Normal => {
            let d = Normal::new(0.5, 0.25).unwrap();
            for _ in 0..cmd.number {
                writeln!(writer, "{}", generate(&regex, &mut r, &d)).unwrap();
            }
        }
        Distribution::Incremental => {
            for mut i in 0..cmd.number {
                writeln!(writer, "{}", generate_inc(&regex, &mut i)).unwrap();
            }
        }
    }
}
