// web/bridge.js — Bridge between WebSocket server and WASM game module.
// Captures local inputs → sends to server.
// Receives rival inputs → injects into WASM.
// Detects game over → notifies Lobby.

const Bridge = (() => {
  // Production URL: replace with actual Render URL once deployed.
  // For local dev, set window.HS_SERVER_URL = 'ws://localhost:3000' in shell.html before this loads.
  const WS_URL = window.HS_SERVER_URL || 'wss://headsoccer-qepm.onrender.com';

  let ws         = null;
  let playerId   = null;   // 1 = host (P1), 2 = guest (P2)
  let gameActive = false;

  // P1 uses WASD, P2 uses arrow keys.
  const KEYS = {
    1: { izq: 'KeyA', der: 'KeyD', salto: 'KeyW', patear: 'KeyS' },
    2: { izq: 'ArrowLeft', der: 'ArrowRight', salto: 'ArrowUp', patear: 'ArrowDown' }
  };

  const held      = new Set();
  let   prevSalto = false;

  document.addEventListener('keydown', e => held.add(e.code));
  document.addEventListener('keyup',   e => held.delete(e.code));

  // 60fps loop: send inputs + detect game over by polling Module._getGanador()
  (function loop() {
    requestAnimationFrame(loop);
    if (!gameActive || !ws || ws.readyState !== WebSocket.OPEN || !playerId) return;

    const K          = KEYS[playerId];
    const saltoHeld  = held.has(K.salto);
    const saltoFlank = saltoHeld && !prevSalto;  // rising edge = only first frame
    prevSalto = saltoHeld;

    ws.send(JSON.stringify({
      type:   'input',
      izq:    held.has(K.izq),
      der:    held.has(K.der),
      salto:  saltoFlank,
      patear: held.has(K.patear)
    }));

    // Poll for 3-goal win condition
    if (typeof Module !== 'undefined' && Module._getGanador) {
      const ganador = Module._getGanador();
      if (ganador !== 0) {
        gameActive = false;
        const p1 = Module._getScoreP1 ? Module._getScoreP1() : 0;
        const p2 = Module._getScoreP2 ? Module._getScoreP2() : 0;
        ws.send(JSON.stringify({ type: 'goal_scored', scorer: ganador }));
        Lobby.onGameOver(p1, p2, playerId);
      }
    }
  })();

  function connect(onOpen) {
    if (ws && ws.readyState <= WebSocket.OPEN) ws.close();
    ws           = new WebSocket(WS_URL);
    ws.onopen    = onOpen;
    ws.onmessage = handleMessage;
    ws.onerror   = () => console.error('[Bridge] WebSocket error');
    ws.onclose   = () => {
      if (gameActive) { gameActive = false; Lobby.onOpponentLeft(); }
    };
  }

  function handleMessage(event) {
    let msg;
    try { msg = JSON.parse(event.data); } catch { return; }
    if (!msg || typeof msg !== 'object') return;

    switch (msg.type) {

      case 'room_created':
        playerId = 1;
        Lobby.onRoomCreated(msg.code);
        break;

      case 'room_joined':
        playerId = 2;
        Lobby.onRoomJoined();
        break;

      case 'opponent_connected':
        Lobby.onOpponentConnected();
        break;

      case 'game_start':
        Lobby.onGameStart(msg.mode);
        break;

      case 'input_relay':
        // Inject rival inputs into WASM. Uses int (not bool) to cross the WASM/JS boundary safely.
        if (typeof Module !== 'undefined' && Module._inyectarInputRival) {
          Module._inyectarInputRival(
            msg.izq    ? 1 : 0,
            msg.der    ? 1 : 0,
            msg.salto  ? 1 : 0,
            msg.patear ? 1 : 0
          );
        }
        break;

      case 'time_up':
        if (!gameActive) break;   // already ended by _getGanador() path
        gameActive = false;
        Lobby.onGameOver(msg.p1Score, msg.p2Score, playerId);
        break;

      case 'rematch_ready':
        Lobby.onRematchReady(msg.mode);
        break;

      case 'opponent_left':
        gameActive = false;
        Lobby.onOpponentLeft();
        break;

      case 'error':
        Lobby.onJoinError(msg.message);
        break;
    }
  }

  return {
    createRoom()   { connect(() => ws.send(JSON.stringify({ type: 'create_room' }))); },
    joinRoom(code) { connect(() => ws.send(JSON.stringify({ type: 'join_room', code }))); },
    selectMode(mode) { ws?.send(JSON.stringify({ type: 'select_mode', mode })); },
    sendRematch()  { ws?.send(JSON.stringify({ type: 'rematch' })); },

    startGame(mode) {
      if (typeof Module !== 'undefined' && Module._iniciarModoRed) {
        Module._iniciarModoRed(mode, playerId);
      }
      gameActive = true;
      prevSalto  = false;
      held.clear();
    }
  };
})();

window.Bridge = Bridge;
