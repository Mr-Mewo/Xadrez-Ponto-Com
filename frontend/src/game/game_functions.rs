use crate::game::piece::Pos;
use std::str::FromStr;
use std::sync::Mutex;
use chess::{Board, ChessMove, MoveGen};
use once_cell::sync::Lazy;
use wasm_bindgen::JsCast;
use web_sys::{window, Element, HtmlElement};

#[allow(unused_imports)]
use crate::{log, get_square_position};
use crate::game::get_piece_props_from_id;
use crate::game::piece::PieceProps;

pub static BOARD: Lazy<Mutex<Board>> = Lazy::new(|| Mutex::new(Board::default()));
pub fn get_board() -> Board { *BOARD.lock().unwrap() }


/*
 * Why explain this?
 * RUST ASS SHENANIGANS!!!
 * Google "Rust mutable reference" for more info
 *
 * I'm such a good teacher, ain't I?
 */
pub fn move_it(m: &str) -> Result<(), chess::Error>{
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

    // Dealing with castling
    let rook_new_position = match (chess_move.get_source(), chess_move.get_dest()) {
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

    if !rook_new_position.0.is_empty() {
        let rook_props = get_piece_props_from_id(rook_new_position.0).unwrap();
        rook_props.prp_sqr_id.set(rook_new_position.1.to_string());
        rook_props.prp_pos.set(get_square_position!(rook_new_position.1));
    }


    Ok(())


    //TODO:
    // OK(dome somewhere else) - Move validation
    // - Send to the server to ask for a move
}

pub fn get_class_at_position(class: &str, x: i32, y: i32) -> Option<Element> {
    window()?.document()?.elements_from_point(x as f32, y as f32)
        .iter()
        .find_map(|el| {
            el.dyn_ref::<Element>()
                .filter(|e| e.class_name().contains(class))
                .cloned()
        })
}

pub fn get_piece_at_square(id: &str) -> Option<HtmlElement> {
    let id = format!("p-{}", id);

    let doc = window().unwrap().document().unwrap();
    let element = doc.get_element_by_id(&id)?;

    element.dyn_into::<HtmlElement>().ok()
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
