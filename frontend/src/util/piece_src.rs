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
