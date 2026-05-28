const COLORES = ['ROJO','AZUL','VERDE','NEGRO','BLANCO','GRIS','GOLD','CYAN'];

function generateCode(existingCodes) {
  let code, attempts = 0;
  do {
    const color = COLORES[Math.floor(Math.random() * COLORES.length)];
    const num   = String(Math.floor(Math.random() * 9000) + 1000);
    code = `${color}-${num}`;
    if (++attempts > 100) throw new Error('No se pudo generar código único');
  } while (existingCodes.includes(code));
  return code;
}

function isValidCode(code) {
  return typeof code === 'string' && /^[A-Z]+-\d{4}$/.test(code);
}

module.exports = { generateCode, isValidCode };
