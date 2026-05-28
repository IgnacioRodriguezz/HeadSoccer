// web/lobby.js — Manages the lobby screens.
// Communicates with bridge.js via window.Bridge (callbacks on the Lobby object).

const MODES = ['Clásico', 'Lunar', 'Ártico'];

const Lobby = (() => {
  let selectedMode = 0;
  let countdownIv  = null;

  const qs  = (id) => document.getElementById(id);
  const err = (id, msg) => { qs(id).textContent = msg; setTimeout(() => qs(id).textContent = '', 3000); };

  function show(id) {
    document.querySelectorAll('.lobby-screen').forEach(s => s.classList.remove('active'));
    qs(`screen-${id}`)?.classList.add('active');
  }

  function init() {
    const overlay = qs('lobby-overlay');
    overlay.innerHTML = `
      <div class="lobby-screen active" id="screen-home">
        <div class="lobby-title">⚽ HeadSoccer</div>
        <p class="lobby-sub">1v1 online · elegí tu rival</p>
        <button class="btn btn-primary"    onclick="Lobby.onCreateRoom()">Crear sala</button>
        <button class="btn btn-secondary"  onclick="Lobby.showJoin()">Unirse con código</button>
      </div>

      <div class="lobby-screen" id="screen-join">
        <div class="lobby-title">Unirse</div>
        <input class="code-input" id="join-input" placeholder="ROJO-1234" maxlength="10"/>
        <div class="error-msg" id="join-error"></div>
        <button class="btn btn-primary"   onclick="Lobby.onJoinRoom()">Conectar</button>
        <button class="btn btn-secondary" onclick="Lobby.showHome()">Volver</button>
      </div>

      <div class="lobby-screen" id="screen-waiting-host">
        <div class="lobby-title">Sala creada</div>
        <p class="lobby-sub">Compartí este código</p>
        <div class="code-display" id="room-code-display">----</div>
        <div class="spinner"></div>
        <p class="lobby-sub">Esperando al rival…</p>
      </div>

      <div class="lobby-screen" id="screen-mode">
        <div class="lobby-title">Elegí el modo</div>
        <div class="mode-grid" id="mode-grid"></div>
        <button class="btn btn-primary" onclick="Lobby.onConfirmMode()">¡Jugar!</button>
      </div>

      <div class="lobby-screen" id="screen-waiting-guest">
        <div class="lobby-title">Conectado</div>
        <div class="spinner"></div>
        <p class="lobby-sub">El host está eligiendo modo…</p>
      </div>

      <div class="lobby-screen" id="screen-countdown">
        <div class="countdown" id="countdown-num">3</div>
      </div>

      <div class="lobby-screen" id="screen-result">
        <div class="lobby-title">Fin de partida</div>
        <div class="result-score"  id="result-score"></div>
        <div class="result-winner" id="result-winner"></div>
        <button class="btn btn-primary"   onclick="Lobby.onRematch()">Revancha</button>
        <button class="btn btn-secondary" onclick="Lobby.showHome()">Salir</button>
      </div>

      <div class="lobby-screen" id="screen-disconnected">
        <div class="lobby-title">😔 Rival desconectado</div>
        <button class="btn btn-primary" onclick="Lobby.showHome()">Volver al inicio</button>
      </div>
    `;

    // Build mode cards
    const grid = qs('mode-grid');
    MODES.forEach((name, i) => {
      const card = document.createElement('div');
      card.className = 'mode-card' + (i === 0 ? ' selected' : '');
      card.innerHTML = `<h3>${name}</h3>`;
      card.onclick   = () => {
        document.querySelectorAll('.mode-card').forEach(c => c.classList.remove('selected'));
        card.classList.add('selected');
        selectedMode = i;
      };
      grid.appendChild(card);
    });

    overlay.classList.remove('hidden');
    show('home');
  }

  function countdown(callback) {
    if (countdownIv) { clearInterval(countdownIv); countdownIv = null; }
    show('countdown');
    let n = 3;
    qs('countdown-num').textContent = n;
    countdownIv = setInterval(() => {
      n--;
      if (n > 0) { qs('countdown-num').textContent = n; }
      else       { clearInterval(countdownIv); countdownIv = null; callback(); }
    }, 1000);
  }

  return {
    init,
    showHome() { show('home'); qs('lobby-overlay').classList.remove('hidden'); },
    showJoin() { show('join'); },

    onCreateRoom()  { window.Bridge?.createRoom(); },
    onJoinRoom()    {
      const code = qs('join-input').value.trim().toUpperCase();
      if (!code) { err('join-error', 'Ingresá un código'); return; }
      window.Bridge?.joinRoom(code);
    },
    onConfirmMode() { window.Bridge?.selectMode(selectedMode); },
    onRematch()     { window.Bridge?.sendRematch(); },

    // ── Callbacks from Bridge ─────────────────────────────────────────────────
    onRoomCreated(code) {
      qs('room-code-display').textContent = code;
      show('waiting-host');
    },
    onRoomJoined()         { show('waiting-guest'); },
    onJoinError(msg)       { err('join-error', msg); },
    onOpponentConnected()  { show('mode'); },

    onGameStart(mode) {
      countdown(() => {
        qs('lobby-overlay').classList.add('hidden');
        window.Bridge?.startGame(mode);
      });
    },

    onGameOver(p1, p2, playerIdLocal) {
      p1 = Number(p1); p2 = Number(p2);
      qs('result-score').textContent = `${p1} — ${p2}`;
      let winner;
      if (p1 === p2) winner = '🤝 Empate';
      else if (p1 > p2) winner = playerIdLocal === 1 ? '🏆 ¡Ganaste!' : '💀 Perdiste';
      else              winner = playerIdLocal === 2 ? '🏆 ¡Ganaste!' : '💀 Perdiste';
      qs('result-winner').textContent = winner;
      show('result');
      qs('lobby-overlay').classList.remove('hidden');
    },

    onRematchReady(mode) {
      countdown(() => {
        qs('lobby-overlay').classList.add('hidden');
        window.Bridge?.startGame(mode);
      });
    },

    onOpponentLeft() {
      show('disconnected');
      qs('lobby-overlay').classList.remove('hidden');
    }
  };
})();

document.addEventListener('DOMContentLoaded', () => Lobby.init());
