const {
  createRoom, joinRoom, getRoom, getRoomByPlayer,
  getPlayerIdInRoom, removeRoom, _rooms
} = require('../roomManager');

const mockWs = () => ({ send: jest.fn(), readyState: 1 });

beforeEach(() => {
  Object.keys(_rooms).forEach(k => delete _rooms[k]);
});

describe('roomManager', () => {
  test('createRoom genera sala con P1 y estado waiting', () => {
    const ws1  = mockWs();
    const room = createRoom(ws1);
    expect(room.code).toMatch(/^[A-Z]+-\d{4}$/);
    expect(room.players[1]).toBe(ws1);
    expect(room.players[2]).toBeNull();
    expect(room.state).toBe('waiting');
  });

  test('joinRoom agrega P2 y cambia estado a ready', () => {
    const ws1 = mockWs(), ws2 = mockWs();
    const room = createRoom(ws1);
    const joined = joinRoom(room.code, ws2);
    expect(joined.players[2]).toBe(ws2);
    expect(joined.state).toBe('ready');
  });

  test('joinRoom lanza error si sala no existe', () => {
    expect(() => joinRoom('FAKE-0000', mockWs())).toThrow('Sala no encontrada');
  });

  test('joinRoom lanza error si sala está llena', () => {
    const ws1 = mockWs(), ws2 = mockWs(), ws3 = mockWs();
    const room = createRoom(ws1);
    joinRoom(room.code, ws2);
    expect(() => joinRoom(room.code, ws3)).toThrow('Sala llena');
  });

  test('getRoom retorna sala correcta', () => {
    const room = createRoom(mockWs());
    expect(getRoom(room.code)).toBe(room);
  });

  test('removeRoom elimina la sala', () => {
    const room = createRoom(mockWs());
    removeRoom(room.code);
    expect(getRoom(room.code)).toBeUndefined();
  });

  test('getRoomByPlayer retorna sala del jugador', () => {
    const ws1 = mockWs();
    const room = createRoom(ws1);
    expect(getRoomByPlayer(ws1)).toBe(room);
  });

  test('getPlayerIdInRoom retorna 1 o 2 correctamente', () => {
    const ws1 = mockWs(), ws2 = mockWs();
    const room = createRoom(ws1);
    joinRoom(room.code, ws2);
    expect(getPlayerIdInRoom(room, ws1)).toBe(1);
    expect(getPlayerIdInRoom(room, ws2)).toBe(2);
  });
});
