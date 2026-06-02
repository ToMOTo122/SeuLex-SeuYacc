// ========== MiniC Lexer (based on minic.l / SeuLex) ==========

const KEYWORDS = {
  int: 'INT', if: 'IF', else: 'ELSE', return: 'RETURN',
  float: 'FLOAT', struct: 'STRUCT', void: 'VOID', while: 'WHILE', for: 'FOR',
};

const SINGLE_CHARS = {
  '(': 'LPAR', ')': 'RPAR',
  '{': 'LBRACE', '}': 'RBRACE',
  '[': 'LBRACK', ']': 'RBRACK',
  ';': 'SEMICOLON', ',': 'COMMA', '.': 'DOT',
  '+': 'PLUS', '-': 'MINUS', '*': 'TIMES', '/': 'DIVIDE', '%': 'MOD',
};

export function lexer(src) {
  const tokens = [];
  let i = 0;
  let line = 1;

  while (i < src.length) {
    const c = src[i];

    // Skip whitespace
    if (c === ' ' || c === '\t' || c === '\r') { i++; continue; }
    if (c === '\n') { i++; line++; continue; }

    // Skip comments
    if (c === '/' && src[i + 1] === '/') {
      while (i < src.length && src[i] !== '\n') i++;
      continue;
    }
    if (c === '/' && src[i + 1] === '*') {
      i += 2;
      while (i < src.length - 1 && !(src[i] === '*' && src[i + 1] === '/')) {
        if (src[i] === '\n') line++;
        i++;
      }
      i += 2;
      continue;
    }

    // Multi-char operators
    if (c === '=' && src[i + 1] === '=') {
      tokens.push({ type: 'EQUAL', value: '==', pos: i, line });
      i += 2; continue;
    }
    if (c === '!' && src[i + 1] === '=') {
      tokens.push({ type: 'NE', value: '!=', pos: i, line });
      i += 2; continue;
    }
    if (c === '<' && src[i + 1] === '=') {
      tokens.push({ type: 'LE', value: '<=', pos: i, line });
      i += 2; continue;
    }
    if (c === '>' && src[i + 1] === '=') {
      tokens.push({ type: 'GE', value: '>=', pos: i, line });
      i += 2; continue;
    }
    if (c === '<') {
      tokens.push({ type: 'LT', value: '<', pos: i, line });
      i++; continue;
    }
    if (c === '>') {
      tokens.push({ type: 'GT', value: '>', pos: i, line });
      i++; continue;
    }
    if (c === '=') {
      tokens.push({ type: 'ASSIGN', value: '=', pos: i, line });
      i++; continue;
    }

    // Identifiers & keywords
    if (/[a-zA-Z_]/.test(c)) {
      let w = '';
      let j = i;
      while (j < src.length && /[a-zA-Z0-9_]/.test(src[j])) { w += src[j]; j++; }
      const type = KEYWORDS[w] || 'NAME';
      tokens.push({ type, value: w, pos: i, line });
      i = j; continue;
    }

    // Numbers (int and float)
    if (/[0-9]/.test(c)) {
      let n = '';
      let j = i;
      while (j < src.length && /[0-9.]/.test(src[j])) { n += src[j]; j++; }
      const type = n.includes('.') ? 'FLOAT_LIT' : 'NUMBER';
      tokens.push({ type, value: n, pos: i, line });
      i = j; continue;
    }

    // Single char operators/delimiters
    if (SINGLE_CHARS[c]) {
      tokens.push({ type: SINGLE_CHARS[c], value: c, pos: i, line });
      i++; continue;
    }

    // Unknown
    tokens.push({ type: 'ERROR', value: c, pos: i, line });
    i++;
  }

  tokens.push({ type: '$', value: '$', pos: i, line });
  return tokens;
}

// Token display helpers
export function tokenDisplay(t) {
  return t.type === '$' ? '$' : t.value;
}

// Token colors - emerald-based palette
export function tokenColor(type) {
  const map = {
    // Keywords - coral/red
    INT: 'bg-rose-100 text-rose-700 border border-rose-200',
    IF: 'bg-rose-100 text-rose-700 border border-rose-200',
    ELSE: 'bg-rose-100 text-rose-700 border border-rose-200',
    RETURN: 'bg-rose-100 text-rose-700 border border-rose-200',
    FLOAT: 'bg-rose-100 text-rose-700 border border-rose-200',
    STRUCT: 'bg-rose-100 text-rose-700 border border-rose-200',
    VOID: 'bg-rose-100 text-rose-700 border border-rose-200',
    WHILE: 'bg-rose-100 text-rose-700 border border-rose-200',
    FOR: 'bg-rose-100 text-rose-700 border border-rose-200',

    // Identifiers - emerald
    NAME: 'bg-emerald-100 text-emerald-700 border border-emerald-200',

    // Literals - amber
    NUMBER: 'bg-amber-100 text-amber-700 border border-amber-200',
    FLOAT_LIT: 'bg-amber-100 text-amber-700 border border-amber-200',

    // Operators - green
    PLUS: 'bg-green-100 text-green-700 border border-green-200',
    MINUS: 'bg-green-100 text-green-700 border border-green-200',
    TIMES: 'bg-green-100 text-green-700 border border-green-200',
    DIVIDE: 'bg-green-100 text-green-700 border border-green-200',
    MOD: 'bg-green-100 text-green-700 border border-green-200',

    // Relational/Assignment - indigo
    ASSIGN: 'bg-indigo-100 text-indigo-700 border border-indigo-200',
    EQUAL: 'bg-indigo-100 text-indigo-700 border border-indigo-200',
    NE: 'bg-indigo-100 text-indigo-700 border border-indigo-200',
    LT: 'bg-indigo-100 text-indigo-700 border border-indigo-200',
    GT: 'bg-indigo-100 text-indigo-700 border border-indigo-200',
    LE: 'bg-indigo-100 text-indigo-700 border border-indigo-200',
    GE: 'bg-indigo-100 text-indigo-700 border border-indigo-200',

    // Delimiters - slate
    LPAR: 'bg-slate-100 text-slate-700 border border-slate-200',
    RPAR: 'bg-slate-100 text-slate-700 border border-slate-200',
    LBRACE: 'bg-slate-100 text-slate-700 border border-slate-200',
    RBRACE: 'bg-slate-100 text-slate-700 border border-slate-200',
    LBRACK: 'bg-slate-100 text-slate-700 border border-slate-200',
    RBRACK: 'bg-slate-100 text-slate-700 border border-slate-200',
    SEMICOLON: 'bg-slate-100 text-slate-700 border border-slate-200',
    COMMA: 'bg-slate-100 text-slate-700 border border-slate-200',
    DOT: 'bg-slate-100 text-slate-700 border border-slate-200',

    // Special
    $: 'bg-emerald-100 text-emerald-700 border border-emerald-200',
    ERROR: 'bg-red-200 text-red-900 border border-red-300',
  };
  return map[type] || 'bg-slate-100 text-slate-700 border border-slate-200';
}

// Token category for grouping
export function tokenCategory(type) {
  if (['INT', 'IF', 'ELSE', 'RETURN', 'FLOAT', 'STRUCT', 'VOID', 'WHILE', 'FOR'].includes(type)) return '关键字';
  if (type === 'NAME') return '标识符';
  if (['NUMBER', 'FLOAT_LIT'].includes(type)) return '常量';
  if (['PLUS', 'MINUS', 'TIMES', 'DIVIDE', 'MOD'].includes(type)) return '算术运算符';
  if (['ASSIGN', 'EQUAL', 'NE', 'LT', 'GT', 'LE', 'GE'].includes(type)) return '关系/赋值';
  if (['LPAR', 'RPAR', 'LBRACE', 'RBRACE', 'LBRACK', 'RBRACK', 'SEMICOLON', 'COMMA', 'DOT'].includes(type)) return '界符';
  if (type === '$') return '结束符';
  return '其他';
}