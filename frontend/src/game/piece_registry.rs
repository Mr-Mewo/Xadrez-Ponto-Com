use std::collections::HashMap;
use std::cell::RefCell;
use std::fmt::format;
use web_sys::HtmlElement;
use yew::UseStateHandle;
use crate::game::piece::{PieceProps, Pos};

// Thread-local registry since Yew runs in single-threaded WebAssembly
thread_local! {
    static PIECE_REGISTRY: RefCell<HashMap<String, PiecePropsRef>> = RefCell::new(HashMap::new());
}

#[derive(Clone)]
pub struct PiecePropsRef {
    pub sqr_id: String,
    pub piece_char: char,
    pub prp_sqr_id: UseStateHandle<String>,
    pub prp_pos: UseStateHandle<Pos>,
}

impl From<&PieceProps> for PiecePropsRef {
    fn from(props: &PieceProps) -> Self {
        Self {
            sqr_id: props.sqr_id.clone(),
            piece_char: props.piece_char,
            prp_sqr_id: props.prp_sqr_id.clone(),
            prp_pos: props.prp_pos.clone(),
        }
    }
}

pub fn register_piece(props: &PieceProps) {
    let piece_id = format!("p-{}", props.sqr_id);
    PIECE_REGISTRY.with(|registry| {
        registry.borrow_mut().insert(piece_id, props.into());
    });
}

pub fn get_piece_props_from_id(id: &str) -> Option<PiecePropsRef> {
    let id = format!("p-{}", id);
    PIECE_REGISTRY.with(|registry| {
        registry.borrow().get(id.as_str()).cloned()
    })
}