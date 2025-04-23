use std::env;
use rusty_comet::scan_file;

fn main() {
    let args: Vec<String> = env::args().skip(1).collect();
    for argument in args {
        scan_file(&argument);
    }
}
