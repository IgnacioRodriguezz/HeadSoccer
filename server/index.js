const { WebSocketServer } = require('ws');
const {
  createRoom, joinRoom, getRoom, getRoomByPlayer,
  getPlayerIdInRoom, removeRoom
} = require('./roomManager');
const { isValidCode } = require('./codeGenerator');

const PORT           = process.env.PORT || 3000;
const MATCH_DURATION = 3 * 60 * 1000;   // 3 minutes in ms
const wss            = new WebSocketServer({ port: PORT });

function send(ws, obj) {
  if (ws?.readyState === 1) ws.send(JSON.stringify(obj));
}

function broadcast(room, obj) {
  send(room.players[1], obj);
  send(room.players[2], obj);
}

wss.on('connection', (ws) => {
  ws.on('message', (raw) => {
    let msg;
    try { msg = JSON.parse(raw); } catch { return; }
    if (!msg || typeof msg !== 'object') return;

    switch (msg.type) {

      case 'create_room': {
        if (getRoomByPlayer(ws)) { send(ws, { type: 'error', message: 'Ya estás en una sala' }); break; }
        try {
          const room = createRoom(ws);
          send(ws, { type: 'room_created', code: room.code, playerId: 1 });
        } catch (e) {
          send(ws, { type: 'error', message: 'No se pudo crear la sala' });
        }
        break;
      }

      case 'join_room': {
        const code = String(msg.code || '').toUpperCase();
        if (!isValidCode(code)) { send(ws, { type: 'error', message: 'Código inválido' }); return; }
        if (getRoomByPlayer(ws)) { send(ws, { type: 'error', message: 'Ya estás en una sala' }); return; }
        try {
          const room = joinRoom(code, ws);
          send(ws,              { type: 'room_joined',        playerId: 2 });
          send(room.players[1], { type: 'opponent_connected' });
        } catch (e) {
          send(ws, { type: 'error', message: e.message });
        }
        break;
      }

      case 'select_mode': {
        const room = getRoomByPlayer(ws);
        if (!room || room.state !== 'ready' || room.players[1] !== ws) return;
        const mode = Number(msg.mode);
        if (![0, 1, 2].includes(mode)) return;

        room.mode      = mode;
        room.state     = 'playing';
        room.startTime = Date.now();

        broadcast(room, { type: 'game_start', mode, startsIn: 3 });

        room.timer = setTimeout(() => {
          if (getRoom(room.code)) {
            broadcast(room, { type: 'time_up', p1Score: room.scores[1], p2Score: room.scores[2] });
            room.state = 'finished';
          }
        }, MATCH_DURATION);
        break;
      }

      case 'input': {
        const room = getRoomByPlayer(ws);
        if (!room || room.state !== 'playing') return;
        const rivalId = getPlayerIdInRoom(room, ws) === 1 ? 2 : 1;
        send(room.players[rivalId], {
          type:   'input_relay',
          izq:    !!msg.izq,
          der:    !!msg.der,
          salto:  !!msg.salto,
          patear: !!msg.patear
        });
        break;
      }

      case 'goal_scored': {
        // Client-reported score: both clients can send this. Used only for time_up final score.
        const room = getRoomByPlayer(ws);
        if (!room || room.state !== 'playing') return;
        const scorer = Number(msg.scorer);
        if (scorer === 1 || scorer === 2) room.scores[scorer]++;
        break;
      }

      case 'rematch': {
        const room = getRoomByPlayer(ws);
        if (!room || room.state !== 'finished') return;
        room.rematchVotes.add(ws);
        if (room.rematchVotes.size === 2) {
          room.scores       = { 1: 0, 2: 0 };
          room.state        = 'playing';
          room.startTime    = Date.now();
          room.rematchVotes.clear();
          broadcast(room, { type: 'rematch_ready', mode: room.mode });
          room.timer = setTimeout(() => {
            if (getRoom(room.code)) {
              broadcast(room, { type: 'time_up', p1Score: room.scores[1], p2Score: room.scores[2] });
              room.state = 'finished';
            }
          }, MATCH_DURATION);
        }
        break;
      }
    }
  });

  ws.on('close', () => {
    const room = getRoomByPlayer(ws);
    if (!room) return;
    const rival = room.players[1] === ws ? room.players[2] : room.players[1];
    send(rival, { type: 'opponent_left' });
    removeRoom(room.code);
  });
});

console.log(`HeadSoccer server on port ${PORT}`);
