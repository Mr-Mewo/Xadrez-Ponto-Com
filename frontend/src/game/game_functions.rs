use crate::game::piece::Pos;
use std::str::FromStr;
use std::sync::Mutex;
use chess::{Board, ChessMove, MoveGen};
use once_cell::sync::Lazy;
use wasm_bindgen::JsCast;
use web_sys::{window, Element, HtmlElement};

#[allow(unused_imports)]
use crate::log;
use crate::game::{get_piece_handles, get_square_pos_by_id};

static BOARD: Lazy<Mutex<Board>> = Lazy::new(|| Mutex::new(Board::default()));
pub fn get_board() -> Board { *BOARD.lock().unwrap() }


/*
 * Why explain this?
 * RUST ASS SHENANIGANS!!!
 * Google "Rust mutable reference" for more info
 *
 * I'm such a good teacher, ain't I?
 */
pub fn make_a_move(m: &str) -> Result<(), chess::Error>{
    let mut board = BOARD.lock().unwrap();
    let mut result = *board;

    // Could require a ChessMove as a parameter but...
    // It's easier to ask for a &str than creating an obj at main
    let chess_move = ChessMove::from_str(m)?;

    // Alters the given board through the reference

    if !MoveGen::new_legal(&board).any(|m| m == chess_move) {
        return Err(chess::Error::InvalidSanMove);
    }

    board.make_move(chess_move, &mut result);
    *board = result;
    // Now 'board' at main is the same as 'result'
    // Rust memory management 101 :)


    //TODO:
    // - Send to the server to ask for a move
    // - Pawn promotion
    // - Deal with moving all the pieces through this function
    // - - Allows for a supposed bot to alter the boards state 
    
    
    // let m = (&m[..2], &m[2..]);

    // let piece_prp = get_piece_handles( m.0 ).unwrap();
    // piece_prp.1.set( m.1.to_string() );
    // piece_prp.2.set( get_square_pos_by_id( m.1) );
    

    // Dealing with castling
    let rook_pos = match (chess_move.get_source(), chess_move.get_dest()) {
        // White kingside castle
        (chess::Square::E1, chess::Square::G1) => ("h1", "f1"),
        // White queenside castle
        (chess::Square::E1, chess::Square::C1) => ("a1", "d1"),
        // Black kingside castle
        (chess::Square::E8, chess::Square::G8) => ("h8", "f8"),
        // Black queenside castle
        (chess::Square::E8, chess::Square::C8) => ("a8", "d8"),
        _ => ("", ""),
    };

    if !rook_pos.0.is_empty() {
        let rook_prp = get_piece_handles( rook_pos.0 ).unwrap();
        rook_prp.1.set( rook_pos.1.to_string() );
        rook_prp.2.set( get_square_pos_by_id(rook_pos.1) );
    }

    Ok(())

}

pub fn get_legal_moves(board: &Board, square: &str) -> Vec<String> {
    MoveGen::new_legal(board)
        .map(|m| m.to_string())
        .filter(|s| s.starts_with(square))
        .map(|s| s[2..].to_string())
        .collect::<Vec<String>>()
}

pub fn piece_src(p: char) -> String {
    let folder = "assets/img/pieces/";

    format!("{}{}.png", folder,
        match p {
            // White pieces
            'K'=>"wKing", 'Q'=>"wQueen", 'R'=>"wRook", 'B'=>"wBishop", 'N'=>"wKnight", 'P'=>"wPawn",
            // Black pieces
            'k'=>"bKing", 'q'=>"bQueen", 'r'=>"bRook", 'b'=>"bBishop", 'n'=>"bKnight", 'p'=>"bPawn",

            // Yes, ./err.png os an actual image, so it will show up
            _ => "err"
        }
    )
}
