#![allow(dead_code)]

pub fn notation_to_index(input: String) -> Option<(usize, usize)> {
    let input = input.to_lowercase();
    let input = (input.chars().next().unwrap(), input.chars().next().unwrap());
    let letters = String::from("abcdefgh");

    Some((
        letters.find(input.0).unwrap(),
        input.1.to_digit(10).unwrap() as usize - 1
    ))
}

pub fn index_to_notation(input: (usize, usize)) -> Option<String> {
    let letters = String::from("abcdefgh");

    Some(format!("{}{}",
        letters.chars().nth(input.0).unwrap(),
        input.1 + 1
    ))
}