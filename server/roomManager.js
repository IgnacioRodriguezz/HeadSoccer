const { generateCode } = require('./codeGenerator');

const _rooms = {};

function createRoom(ws1) {
  const code = generateCode(Object.keys(_rooms));
  _rooms[code] = {
    code,
    players:      { 1: ws1, 2: null },
    state:        'waiting',
    mode:         null,
    startTime:    null,
    scores:       { 1: 0, 2: 0 },
    timer:        null,
    rematchVotes: new Set()
  };
  return _rooms[code];
}

function joinRoom(code, ws2) {
  const room = _rooms[code];
  if (!room)            throw new Error('Sala no encontrada');
  if (room.players[2])  throw new Error('Sala llena');
  room.players[2] = ws2;
  room.state      = 'ready';
  return room;
}

function getRoom(code)        { return _rooms[code]; }
function getRoomByPlayer(ws)  { return Object.values(_rooms).find(r => r.players[1] === ws || r.players[2] === ws); }
function getPlayerIdInRoom(room, ws) {
  if (room.players[1] === ws) return 1;
  if (room.players[2] === ws) return 2;
  return null;
}

function removeRoom(code) {
  if (_rooms[code]?.timer) clearTimeout(_rooms[code].timer);
  delete _rooms[code];
}

module.exports = { createRoom, joinRoom, getRoom, getRoomByPlayer, getPlayerIdInRoom, removeRoom, _rooms };
