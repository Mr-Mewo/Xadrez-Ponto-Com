use std::{
    str::FromStr,
    sync::Mutex
};
use once_cell::sync::Lazy;
use chess::{Board, ChessMove, MoveGen};
use crate::game::{
    get_piece_handles, get_square_pos_by_id, piece::PieceProps,
    update_piece_key,
};
use crate::game::get_piece_at_square;
#[allow(unused_imports)]
use crate::log;

static BOARD: Lazy<Mutex<Board>> = Lazy::new(|| Mutex::new(Board::default()));
pub fn get_board() -> Board { *BOARD.lock().unwrap() }

/*
 * Why explain this?
 * RUST ASS SHENANIGANS!!!
 * Google "Rust mutable reference" for more info
 *
 * I'm such a good teacher, ain't I?
 */
pub fn make_move(m: &str, prp_ref: Option<&PieceProps>) -> Result<(), chess::Error>{
    let mut board = BOARD.lock().unwrap();
    let mut result = *board;

    // Could require a ChessMove as a parameter
    // But it's easier to ask for a &str than creating an obj at main
    let chess_move = ChessMove::from_str(m)?;

    // Chess move validation is THAT easy! (Joke btw, it's just because I'm using a lib)
    if !MoveGen::new_legal(&board).any(|m| m == chess_move) {
        return Err(chess::Error::InvalidSanMove);
    }

    board.make_move(chess_move, &mut result);
    *board = result;
    // Now 'board' at main is the same as 'result'
    // Rust memory management 101 :)

    if let Some(eaten_piece) = get_piece_at_square( chess_move.get_dest().to_string().as_str() ) {
        // Places the "eaten" class on the eaten piece
        eaten_piece.set_class_name(format!("{} eaten", eaten_piece.class_name()).as_str());
        eaten_piece.set_id("");
    }

    //TODO:
    // (Somewhere else) Send to the server to ask for a move
    // - Pawn promotion
    // (Ok) Deal with moving all the pieces through this function
    // - - Allows for a supposed bot to alter the boards state
    // (somehow) - en-peasant


    if board.en_passant()
        .is_some_and( |s| {s == chess_move.get_source()} ){
        if let Some(captured_pawn) = get_piece_at_square(&format!("{:?}{:?}", chess_move.get_dest().get_file(), chess_move.get_source().get_rank())) {
            captured_pawn.set_class_name(format!("{} eaten", captured_pawn.class_name()).as_str());
            captured_pawn.set_id("");
        }
    }

    // Castling
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

    // Adds it to the end, gets passed to the function
    let m = format!("{}{}{}", m, rook_pos.0, rook_pos.1);
    piece_position_update(m.as_str(), prp_ref);

    Ok(())
}

// Gets the Option<&PieceProps>
// Skips having to get the handles when I already have them
// Pieces move themselves, they know their own PieceProps
fn piece_position_update(m: &str, prp_ref: Option<&PieceProps>) {
    let (src, dest, xtr) = (&m[..2], &m[2..4], &m[4..]);

    // Search for the piece if it doesn't have it
    let prp = if prp_ref.is_some() {
        #[allow(clippy::unnecessary_unwrap)]
        prp_ref.unwrap()
    }else {
        &get_piece_handles(src).unwrap()
    };

    prp.prp_sqr_id.set( dest.to_string() );
    prp.prp_pos.set( get_square_pos_by_id( dest ) );

    update_piece_key(src, dest);

    // YES GUYS!
    // THIS IS RECURSIVENESS GUYS!
    // GUYS GET ALL HASKELL PROGRAMMERS ON THE LINE!!!
    if !xtr.is_empty() {
        piece_position_update(xtr, None);
    }
}

pub fn get_legal_moves(board: &Board, square: &str) -> Vec<String> {
    MoveGen::new_legal(board)
        .map(|m| m.to_string())
        .filter(|s| s.starts_with(square))
        .map(|s| s[2..].to_string())
        .collect::<Vec<String>>()
}
