use yew::prelude::*;
use crate::custom_tags as ct;

pub const FILE_LETTERS: &str = "abcdefgh";

#[derive(Properties, PartialEq)]
pub struct ChessBoardProps {
    #[prop_or_default]
    pub class: Classes,
    #[prop_or(4)]
    pub pixel_size: u32,
}

#[function_component(ChessBoard)]
pub fn chess_board(props: &ChessBoardProps) -> Html {

    let label = FILE_LETTERS.chars().map(|c| {
        html! {
            <div class="file-label">{ c }</div>
        }
    }).collect::<Html>();

    html! {
        <ct::NineSlice src="9sl/area-rnd-3.png" class={classes!("board", props.class.clone())}>

            {
                (0..8).rev().map(|rank| {
                    html! {
                        <div class="board-row">
                            {
                                (0..8).map(|file| {
                                    let is_light = (rank + file) % 2 == 0;
                                    let square_src = if is_light { "brd/white-2.png" } else { "brd/black-3.png" };
                                    let square_class = if is_light { "square light" } else { "square dark" };
                                    html! {
                                        <ct::NineSlice src={square_src} class={square_class} size={props.pixel_size * 4}></ct::NineSlice>
                                    }
                                }).collect::<Html>()
                            }
                            <div class="rank-label">{ rank + 1 }</div>
                            <div class="rank-label">{ rank + 1 }</div>
                        </div>
                    }
                }).collect::<Html>()
            }

            <div class="file-labels"> { label.clone() } </div>
            <div class="file-labels"> { label } </div>

        </ct::NineSlice>

    }
}