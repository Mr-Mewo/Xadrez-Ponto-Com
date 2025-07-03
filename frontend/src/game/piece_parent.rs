use yew::{function_component, html, Html};
use crate::game::BOARD;
use crate::log;
use crate::game::piece::Piece;

static RANK_LETTERS: &str = "abcdefgh";

#[function_component(PieceParent)]
pub fn piece_parent() -> Html {
    let board = BOARD.lock().unwrap().to_string();
    log!("{}:", board);
    let board = board.split(" ").next().unwrap();

    let mut ranks = 0;
    let mut files = 0;
    let mut pieces: Vec<Html> = Vec::new();

    for c in board.chars() {
        match c {
            '/' => {
                ranks += 1;
                files = 0;
            }

            d if d.is_ascii_digit() => {
                files += d.to_digit(10).unwrap() as usize;
            },

            piece => {
                let rank = RANK_LETTERS.chars().nth(files).unwrap();
                let file = 8 - ranks;
                let square_id = format!("{}{}", rank, file);

                log!("Adding piece {} at {}", piece, square_id);

                pieces.push(html! {
                    <PieceWrapper
                        piece_char={piece}
                        sqr_id={square_id}
                    />
                });

                files += 1;
            },
        }
    }

    pieces.into_iter().collect::<Html>()
}

#[derive(yew::Properties, PartialEq)]
pub struct PieceWrapperProps {
    pub piece_char: char,
    pub sqr_id: String,
}

#[function_component(PieceWrapper)]
pub fn piece_wrapper(props: &PieceWrapperProps) -> Html {
    use yew::use_state;
    use crate::game::piece::Pos;

    let piece_sqr_id = use_state( String::new );
    let piece_pos = use_state(|| Pos::new(0, 0));

    html! {
        <Piece
            piece_char={props.piece_char}
            sqr_id={props.sqr_id.clone()}
            prp_sqr_id={piece_sqr_id}
            prp_pos={piece_pos}
        />
    }
}
