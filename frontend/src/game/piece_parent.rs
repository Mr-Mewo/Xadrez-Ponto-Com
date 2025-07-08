use yew::{function_component, html, Html};
use crate::{
    game::{
        piece::{Piece, PieceProps},
        get_board,
        register_piece
    }
};

#[allow(unused_imports)]
use crate::log;

static RANK_LETTERS: &str = "abcdefgh";

#[function_component(PieceParent)]
pub fn piece_parent() -> Html {
    let board = get_board().to_string();
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

                pieces.push(html! {
                    <PieceWrapper
                        in_piece_char={piece}
                        in_sqr_id={square_id}
                    />
                });

                files += 1;
            },
        }
    }

    html! {
        <div class="piece-parent">
            { pieces.into_iter().collect::<Html>() }
        </div>
    }
}

#[derive(yew::Properties, PartialEq)]
pub struct PieceWrapperProps {
    pub in_piece_char: char,
    pub in_sqr_id: String,
}

#[function_component(PieceWrapper)]
pub fn piece_wrapper(props: &PieceWrapperProps) -> Html {
    use yew::use_state;
    use crate::game::piece::Pos;

    let piece_sqr_id = use_state( String::new );
    let piece_pos = use_state(|| Pos::new(0, 0));

    let prp = PieceProps {
        piece_char: props.in_piece_char,
        sqr_id: props.in_sqr_id.clone(),
        prp_sqr_id: piece_sqr_id,
        prp_pos: piece_pos,
    };

    register_piece(&prp);

    html! {
        <Piece
            piece_char={prp.piece_char}
            sqr_id={prp.sqr_id}
            prp_sqr_id={prp.prp_sqr_id}
            prp_pos={prp.prp_pos}
        />
    }
}
