use std::env;
use std::fs::File;
use std::io::prelude::*;
use std::path::Path;

mod lexer;

use crate::lexer::scan;

fn main() {
    let args: Vec<String> = env::args().skip(1).collect();
    for argument in args {
        let path = Path::new(&argument);
        let display = path.display();

        // Open the path in read-only mode, returns `io::Result<File>`
        let mut file = match File::open(&path) {
            Err(why) => panic!("couldn't open {}: {}", display, why),
            Ok(file) => file,
        };

        // Read the file contents into a string, returns `io::Result<usize>`
        let mut s = String::new();
        match file.read_to_string(&mut s) {
            Err(why) => panic!("couldn't read {}: {}", display, why),
            Ok(_) => print!("{} contains:\n{}", display, s),
        }

        scan(&s);
    }
}
