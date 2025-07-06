use std::string::String;
use yew::prelude::*;
use crate::custom_tags as tags;

const RANK_LETTERS: &str = "abcdefgh";

#[derive(Properties, PartialEq)]
pub struct ChessBoardProps {
    #[prop_or_default]
    pub class: Classes,
    #[prop_or(4)]
    pub pixel_size: u32,

    pub white_src: String,
    pub black_src: String,
}

#[function_component(ChessBoard)]
pub fn chess_board(props: &ChessBoardProps) -> Html {
    let white_src = format!("brd/{}", props.white_src);
    let black_src = format!("brd/{}", props.black_src);

    let rank_labels = RANK_LETTERS.chars().map(|c| {
        html! { <div class="rank-label">{ c }</div> }
    }).collect::<Html>();

    let file_labels = (0..8).rev().map(|c| {
        html! { <div class="file-label">{ { c+1 } }</div> }
    }).collect::<Html>();

    html! {
        <tags::NineSlice src="9sl/area-rnd.png" size={props.pixel_size*12} class={classes!("board", props.class.clone())}>
            {
    // RANKS
                (0..8).map(|file| {
                    html! {
                        <div class="board-row">
                            {
    // FILES
                                RANK_LETTERS.chars().enumerate().map(|(rank, char_rank)| {
                                    let is_light = (rank + file) % 2 == 1;
                                    let src = if is_light { white_src.clone() } else { black_src.clone() };
                                    let class = if is_light { "square light" } else { "square dark" };
                                    let id = format!("{}{}", char_rank, 8-file);
    // SQUARE
                                    html! {
                                        <tags::NineSlice size={props.pixel_size * 4} {src} {class} {id}>
                                            // Internal square elements
                                            <div class="hover"/> // Mouse hover. Yes, what did you expect?
                                            <div class="selected"/> // Clicked, contains a piece? Still thinking about that
                                            <div class="movable-to"/> // As for there is a possible move to this square
                                            <div class="check"/> // Kings square is in check
                                        </tags::NineSlice>
                                    }
                                }).collect::<Html>()
                            }
                        </div>
                    }
                }).collect::<Html>()
            }

            <div class="labels">
                <div class="file left"> { file_labels.clone() } </div>
                <div class="file right"> { file_labels } </div>
                <div class="rank top"> { rank_labels.clone() } </div>
                <div class="rank bottom"> { rank_labels } </div>
            </div>
        </tags::NineSlice>

    }
}
