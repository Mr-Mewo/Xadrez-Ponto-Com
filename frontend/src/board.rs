use yew::prelude::*;
use crate::custom_tags as ct;

pub const FILE_LETTERS: &str = "abcdefgh";

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

    let file_labels = FILE_LETTERS.chars().map(|c| {
        html! {
            <div class="file-label">{ c }</div>
        }
    }).collect::<Html>();
    
    let rank_labels = (0..8).map(|c| {
        html! {
            <div class="rank-label">{ {8 - c} }</div>
        }
    }).collect::<Html>();

    html! {
        <ct::NineSlice src="9sl/area-rnd.png" size={props.pixel_size*12} class={classes!("board", props.class.clone())}>

            {
                (0..8).rev().map(|rank| {
                    html! {
                        <div class="board-row">
                            {
                                (0..8).map(|file| {
                                    let is_light = (rank + file) % 2 == 1;
                                    let src = if is_light { white_src.clone() } else { black_src.clone() };
                                    let class = if is_light { "square light" } else { "square dark" };
                                    html! {
                                        <ct::NineSlice {src} {class} size={props.pixel_size * 4}></ct::NineSlice>
                                    }
                                }).collect::<Html>()
                            }
                        </div>
                    }
                }).collect::<Html>()
            }
        
            <div class="labels">
                <div class="rank left"> { rank_labels.clone() } </div>
                <div class="rank right"> { rank_labels } </div>
                <div class="file top"> { file_labels.clone() } </div>
                <div class="file bottom"> { file_labels } </div>
            </div>
        </ct::NineSlice>

    }
}