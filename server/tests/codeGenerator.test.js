const { generateCode, isValidCode } = require('../codeGenerator');

describe('codeGenerator', () => {
  test('devuelve string con formato PALABRA-XXXX', () => {
    const code = generateCode([]);
    expect(code).toMatch(/^[A-Z]+-\d{4}$/);
  });

  test('no repite códigos ya existentes', () => {
    const existing = ['ROJO-1234', 'AZUL-5678'];
    const code = generateCode(existing);
    expect(existing).not.toContain(code);
  });

  test('isValidCode acepta formato correcto', () => {
    expect(isValidCode('ROJO-1234')).toBe(true);
  });

  test('isValidCode rechaza formatos inválidos', () => {
    expect(isValidCode('rojo-1234')).toBe(false);
    expect(isValidCode('1234-ROJO')).toBe(false);
    expect(isValidCode('')).toBe(false);
    expect(isValidCode(null)).toBe(false);
  });
});
