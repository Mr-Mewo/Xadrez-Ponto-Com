use std::collections::HashMap;
use std::cell::RefCell;
use yew::UseStateHandle;
use crate::game::piece::{PieceProps, Pos};

// Thread-local registry since Yew runs in single-threaded WebAssembly
thread_local! {
    static PIECE_REGISTRY: RefCell<HashMap<String, (char, UseStateHandle<String>, UseStateHandle<Pos>)>>
                            = RefCell::new(HashMap::new());
}

pub fn register_piece(props: &PieceProps) {
    let piece_id = format!("p-{}", props.sqr_id);
    PIECE_REGISTRY.with(|registry| {
        registry.borrow_mut().insert(
            piece_id,
            (props.piece_char, props.prp_sqr_id.clone(), props.prp_pos.clone())
        );
    });
}

pub fn get_piece_handles(id: &str) -> Option<(char, UseStateHandle<String>, UseStateHandle<Pos>)> {
    let piece_id = format!("p-{}", id);
    PIECE_REGISTRY.with(|registry| {
        registry.borrow().get(&piece_id).cloned()
    })
}
