#[macro_export]
macro_rules! get_legal_moves {
    ($board:expr, $square:expr) => {
        MoveGen::new_legal($board) // An array of all available legal moves, based on current board
            .map(|m| m.to_string())
            .filter(|s| s.starts_with($square)) // Filter to moves available for this piece
            .map(|s| s[2..].to_string()) // Keep only the destination square
            .collect::<Vec<String>>() // Convert to a Vector of Strings
    }
}


// Finds the position of the current square
// It's a macro for simplicity
#[macro_export]
macro_rules! get_square_position {
    ($sqr_id:expr) => {{
        let doc = web_sys::window().unwrap().document().unwrap();
        let square = doc.query_selector(&format!("#{}", $sqr_id)).unwrap().unwrap();
        let rect = square.get_bounding_client_rect();
        
        Pos::new((rect.left() + rect.width() / 2.0) as i32,
                 (rect.top() + rect.height() / 2.0) as i32)
    }}
}

#[macro_export]
macro_rules! get_square_size {
    () => {{
        let doc = web_sys::window().unwrap().document().unwrap();
        let square = doc.query_selector(".square").unwrap().unwrap();
        let rect = square.get_bounding_client_rect();
        rect.height() // Squares are always square, so width = height
    }}
}
