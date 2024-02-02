use std::io::{stdout, BufWriter, Write};

use clap::Parser;

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
    pub fn generate(&self) -> char {
        if !self.complement {
            self.inner
                .chars()
                .nth(fastrand::usize(0..self.inner.len()))
                .unwrap()
        } else {
            let mut ch = fastrand::char(..);
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

/// Represents a Repetition (e.g. `{3}`) of values
#[derive(Debug)]
struct Repetitions {
    start: u32,
    comma: bool,
    end: Option<u32>,
}

/// Possible Regex symbols
#[derive(Debug)]
enum RegexSymbol {
    /// the `.` special character
    Any,
    /// stands for the exact character it contains
    Literal(char),
    /// a parenthesized group, e.g. `(inner_regex)`
    Group(Vec<Regex>),
    /// list, e.g. `[0-9]`
    List(List),
}

/// A Regex is a symbol and its possible modifiers
#[derive(Debug)]
struct Regex {
    symbol: RegexSymbol,
    /// "one or more"
    plus: bool,
    /// "zero or more"
    star: bool,
    /// "optional"
    question: bool,
    repetitions: Repetitions,
}

impl Regex {
    /// Initializes a new Regex from a symbol, setting all modifiers to false
    pub fn new(symbol: RegexSymbol) -> Self {
        Self {
            symbol,
            plus: false,
            star: false,
            question: false,
            repetitions: Repetitions {
                start: 1,
                comma: false,
                end: None,
            },
        }
    }

    /// Returns random characters according to the regular expression rules
    pub fn generate(&self) -> String {
        let repetitions = if self.plus {
            fastrand::usize(1..5)
        } else if self.star {
            fastrand::usize(0..5)
        } else if self.question {
            fastrand::usize(0..=1)
        } else {
            let Repetitions { start, comma, end } = self.repetitions;
            if let Some(end) = end {
                fastrand::usize(start as usize..=end as usize)
            } else if comma {
                fastrand::usize(start as usize..start as usize + 5)
            } else {
                start as usize
            }
        };

        let mut s = String::with_capacity(repetitions);
        for _ in 0..repetitions {
            match &self.symbol {
                RegexSymbol::Any => s.push(fastrand::char(..)),
                RegexSymbol::Literal(literal) => s.push(*literal),
                RegexSymbol::Group(group) => s.push_str(&generate(group)),
                RegexSymbol::List(list) => s.push(list.generate()),
            }
        }

        s
    }

    /// Returns incremental characters according to the regular expression rules
    pub fn generate_inc(&self, i: &mut u64) -> String {
        let repetitions = if self.plus {
            fastrand::usize(1..5)
        } else if self.star {
            fastrand::usize(0..5)
        } else if self.question {
            fastrand::usize(0..=1)
        } else {
            let Repetitions { start, comma, end } = self.repetitions;
            if let Some(end) = end {
                fastrand::usize(start as usize..=end as usize)
            } else if comma {
                fastrand::usize(start as usize..start as usize + 5)
            } else {
                start as usize
            }
        };

        let mut s = String::with_capacity(repetitions);
        for _ in 0..repetitions {
            match &self.symbol {
                RegexSymbol::Any => {
                    s.insert(
                        0,
                        char::from_u32(*i as u32 + 32).expect("failed to generate incremental '.'"),
                    );
                    *i = i.saturating_sub(1);
                }
                RegexSymbol::Literal(literal) => s.insert(0, *literal),
                RegexSymbol::Group(group) => s = generate_inc(group, i) + &s,
                RegexSymbol::List(list) => s.insert(0, list.generate_inc(i)),
            }
        }

        s
    }
}

/// generate a random regex
fn generate(regexes: &[Regex]) -> String {
    let mut s = String::with_capacity(regexes.len());
    for regex in regexes {
        s.push_str(&regex.generate());
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
fn parse_repetitions(chars: &mut std::str::Chars) -> Repetitions {
    let mut s = String::new();
    for ch in chars.by_ref() {
        if ch == '}' {
            break;
        }
        s.push(ch);
    }

    let start;
    let mut comma = false;
    let mut end = None;

    match s.split_once(',') {
        None => start = s.trim().parse().unwrap(),
        Some((start_str, end_str)) => {
            start = start_str.trim().parse().unwrap();
            comma = true;
            end = end_str.trim().parse().ok()
        }
    }

    Repetitions { start, comma, end }
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
            '{' => group.last_mut().unwrap().repetitions = parse_repetitions(chars),
            ')' => break,
            '(' => group.push(parse_group(chars)),
            '+' => group.last_mut().unwrap().plus = true,
            '*' => group.last_mut().unwrap().star = true,
            '?' => group.last_mut().unwrap().question = true,
            '.' => group.push(Regex::new(Any)),
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
            '{' => tree.last_mut().unwrap().repetitions = parse_repetitions(&mut chars),
            ')' => panic!("badly formatted regex!"),
            '(' => tree.push(parse_group(&mut chars)),
            '+' => tree.last_mut().unwrap().plus = true,
            '*' => tree.last_mut().unwrap().star = true,
            '?' => tree.last_mut().unwrap().question = true,
            '.' => tree.push(Regex::new(Any)),
            ch => tree.push(Regex::new(Literal(ch))),
        }
    }

    tree
}

#[derive(Parser)]
#[command(version, name = "keygen")]
/// `keygen` generates random strings based on a regex
///
/// Note the OR operator (|) is not implemented
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

    /// Whether to generate regexes in incremental fashion, in order
    ///
    /// For example, normally, a regex like [0-9]{3} would generate random 3 digit numbers. By
    /// passing this flag, the gerator will produce '001', '002', '003', and so on, in order.
    #[clap(short, long)]
    incremental: bool,
}

fn main() {
    let cmd = Command::parse();

    fastrand::seed(cmd.seed);

    let regex = parse_regex(cmd.regex.chars());

    #[cfg(debug_assertions)]
    {
        println!("{regex:#?}");
    }

    let stdout = stdout();
    let lock = stdout.lock();
    let mut writer = BufWriter::new(lock);
    if cmd.incremental {
        for mut i in 0..cmd.number {
            writeln!(writer, "{}", generate_inc(&regex, &mut i)).unwrap();
        }
    } else {
        for _ in 0..cmd.number {
            writeln!(writer, "{}", generate(&regex)).unwrap();
        }
    }
}
