
// Display information when screen element is turned on 
void displayCustom(void)
{
  uint32_t now = millis();

  for (uint8_t idx = 0; idx < NUM_CUSTOM_RECTS; idx++) {
    volatile Rect &r = customRects[idx];
    uint32_t t0      = lastMsgTime[idx];

    if (AIM) {
      MAX7456_SetScreenSymbol(SYM_AIM, 225);
      MAX7456_SetScreenSymbol(SYM_AIM + 1, 226);
      MAX7456_SetScreenSymbol(SYM_AIM + 2, 255);
      MAX7456_SetScreenSymbol(SYM_AIM + 3, 256);
      continue;
    }

    if (r.type == 'n') continue;

    if (t0 == 0) continue;

    if (r.x >= COLS) continue;
    if (r.y >= ROWS) continue;

    uint16_t base = uint16_t(r.y) * COLS + r.x;
    bool expired = 0;
    if (r.x == 0 && r.y == 0 && r.h == 0 && r.w == 0) expired = 1;

    uint8_t smallType = r.cls;
    if (smallType > 4) smallType = 0;

    // --- отрисовка ---
    if (r.w < 2 && r.h >= 2) {
      if (base < COLS * ROWS)
        // MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? DOWN_STRETCHED_SQUARE_UP : (uint8_t)(CROP_DOWN_STRETCHED_SQUARE_UP + smallType)), base);
        MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? DOWN_STRETCHED_SQUARE_UP : CROP_DOWN_STRETCHED_SQUARE), base);
      for (uint8_t dy = 1; dy + 1 < r.h; dy++) {
        uint16_t rowBase = uint16_t(r.y + dy) * COLS;
        uint16_t pos = rowBase + r.x;
        if (pos < COLS * ROWS) 
          MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? (uint8_t)(DOWN_STRETCHED_SQUARE_UP + 1) : (uint8_t)(CROP_DOWN_STRETCHED_SQUARE + 1)), pos);
      }
      uint16_t botBase = base + uint16_t(r.h - 1) * COLS;
      if (botBase < COLS * ROWS) 
        MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? (uint8_t)(DOWN_STRETCHED_SQUARE_UP + 2) : (uint8_t)(CROP_DOWN_STRETCHED_SQUARE + 2)), botBase);
    }
    else if (r.w >= 2 && r.h < 2) {
      if (base < COLS * ROWS)
        // MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? RIGHT_STRETCHED_SQUARE_LEFT : (uint8_t)(CROP_RIGHT_STRETCHED_SQUARE_LEFT + smallType)), base);
        MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? RIGHT_STRETCHED_SQUARE_LEFT : CROP_RIGHT_STRETCHED_SQUARE), base);
      for (uint8_t dx = 1; dx + 1 < r.w; dx++) {
        uint16_t pos = base + dx;
        if (pos < COLS * ROWS)
          MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? (uint8_t)(RIGHT_STRETCHED_SQUARE_LEFT + 1) : (uint8_t)(CROP_RIGHT_STRETCHED_SQUARE + 1)), pos);
      }
      if (base + r.w - 1 < COLS * ROWS) 
        MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? (uint8_t)(RIGHT_STRETCHED_SQUARE_LEFT + 2) : (uint8_t)(CROP_RIGHT_STRETCHED_SQUARE + 2)), base + r.w - 1);
    }
    else if (r.w < 2 && r.h < 2) {
      if (base < COLS * ROWS)
        MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? PIXEL_SQUARE : CROP_PIXEL_SQUARE), base);
      /*if (!expired && r.y > 0 && r.type == 'd') {
        uint16_t iconPos = uint16_t(r.y - 1) * COLS + r.x;
        if (iconPos < COLS * ROWS) {
          if (smallType == 0) MAX7456_SetScreenSymbol(TANK, iconPos);
          else if (smallType == 1) MAX7456_SetScreenSymbol(LBT, iconPos);
          else if (smallType == 2) MAX7456_SetScreenSymbol(AUTO, iconPos);
          else if (smallType == 3) MAX7456_SetScreenSymbol(HUMAN, iconPos);
          else if (smallType == 4) MAX7456_SetScreenSymbol(KILLED, iconPos);
        }
      }*/
    }
    else {
      if (base < COLS * ROWS)
        // MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? SQUARE_UL : (uint8_t)(CROP_SQUARE_UL + smallType)), base);
        MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? SQUARE_UL : CROP_SQUARE), base);
      for (uint8_t dx = 1; dx + 1 < r.w; dx++) {
        uint16_t pos = base + dx;
        if (pos < COLS * ROWS)
          MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? (uint8_t)(SQUARE_UL + 1) : (uint8_t)(CROP_SQUARE + 1)), pos);
      }
      if (base + r.w - 1 < COLS * ROWS) 
        MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? (uint8_t)(SQUARE_UL + 2) : (uint8_t)(CROP_SQUARE + 2)), base + r.w - 1);

      for (uint8_t dy = 1; dy + 1 < r.h; dy++) {
        uint16_t rowBase = uint16_t(r.y + dy) * COLS;
        if (rowBase + r.x < COLS * ROWS)
          MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? SQUARE_SL : CROP_SQUARE_SL), rowBase + r.x);
        if (rowBase + r.x + r.w - 1 < COLS * ROWS)
          MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? SQUARE_SR : CROP_SQUARE_SR), rowBase + r.x + r.w - 1);
      }

      uint16_t botBase = base + uint16_t(r.h - 1) * COLS;
      if (botBase < COLS * ROWS)
        MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? (SQUARE_UL + 3) : (CROP_SQUARE + 3)), botBase);
      for (uint8_t dx = 1; dx + 1 < r.w; dx++) {
        uint16_t pos = botBase + dx;
        if (pos < COLS * ROWS) 
          MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? (SQUARE_UL + 4) : (CROP_SQUARE + 4)), pos);
      }
      if (r.w > 1 && botBase + r.w - 1 < COLS * ROWS)
        MAX7456_SetScreenSymbol(expired ? SYM_BLANK : (r.line ? (SQUARE_UL + 5) : (CROP_SQUARE + 5)), botBase + r.w - 1);
    }
    
  } // for

  displayReady = true;
}
