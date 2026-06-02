// ========== MiniC Parser (recursive descent based on minic.y / SeuYacc) ==========

export function parse(tokens) {
  const events = [];
  let ti = 0;

  function pk() { return tokens[ti] || { type: '$', value: '$' }; }
  function sh() {
    const t = pk();
    ti++;
    events.push({ action: 'shift', token: t, tokenIndex: ti - 1 });
    return t;
  }
  function rd(text, n) {
    events.push({ action: 'reduce', text, symbolCount: n });
  }
  function err(m) {
    events.push({ action: 'error', text: m });
  }

  function isFunction() {
    for (let j = ti; j < tokens.length - 2; j++) {
      if (['INT', 'FLOAT', 'STRUCT', 'VOID'].indexOf(tokens[j].type) >= 0
        && tokens[j + 1] && tokens[j + 1].type === 'NAME'
        && tokens[j + 2] && tokens[j + 2].type === 'LPAR') return true;
      break;
    }
    return false;
  }

  // ---- Grammar functions ----
  function pType() {
    const t = pk();
    if (t.type === 'INT') { sh(); rd('type → INT', 1); return 1; }
    if (t.type === 'FLOAT') { sh(); rd('type → FLOAT', 1); return 1; }
    if (t.type === 'VOID') { sh(); rd('type → VOID', 1); return 1; }
    if (t.type === 'STRUCT') {
      sh();
      if (pk().type !== 'LBRACE') { err('期望 {'); return 0; }
      sh();
      pFields();
      if (pk().type !== 'RBRACE') { err('期望 }'); return 0; }
      sh();
      rd('type → struct { fields }', 4);
      return 4;
    }
    err('期望类型');
    return 0;
  }

  function pFields() {
    if (['INT', 'FLOAT', 'STRUCT'].indexOf(pk().type) >= 0) {
      pField();
      pFields();
      rd('fields → field fields', 2);
    } else {
      rd('fields → ε', 0);
    }
  }

  function pField() {
    pType();
    if (pk().type !== 'NAME') { err('期望 NAME'); return; }
    sh();
    if (pk().type !== 'SEMICOLON') { err('期望 ;'); return; }
    sh();
    rd('field → type NAME ;', 3);
  }

  function pVarDecl() {
    pType();
    if (pk().type !== 'NAME') { err('期望 NAME'); return; }
    sh();
    if (pk().type !== 'SEMICOLON') { err('期望 ;'); return; }
    sh();
    rd('var_decl → type NAME ;', 3);
  }

  function pVarDecls() {
    let n = 0;
    while (['INT', 'FLOAT', 'STRUCT'].indexOf(pk().type) >= 0 && !isFunction()) {
      pVarDecl();
      n++;
    }
    if (n) rd('var_decls → var_decl+', n);
    else rd('var_decls → ε', 0);
  }

  function pFunDecl() {
    pFunHeader();
    if (pk().type !== 'LPAR') { err('期望 ('); return; }
    sh();
    pParams();
    if (pk().type !== 'RPAR') { err('期望 )'); return; }
    sh();
    pBlock();
    rd('fun_decl → fun_header ( params ) block', 5);
  }

  function pFunHeader() {
    pType();
    if (pk().type !== 'NAME') { err('期望 NAME'); return; }
    sh();
    rd('fun_header → type NAME', 2);
  }

  function pParams() {
    if (['INT', 'FLOAT', 'STRUCT'].indexOf(pk().type) >= 0
      && tokens[ti + 1] && tokens[ti + 1].type === 'NAME') {
      pParam();
      pMoreParams();
      rd('params → param more_params', 2);
    } else {
      rd('params → ε', 0);
    }
  }

  function pMoreParams() {
    pParam();
    if (pk().type === 'COMMA') {
      sh();
      pMoreParams();
      rd('more_params → param , more_params', 3);
    } else {
      rd('more_params → param', 1);
    }
  }

  function pParam() {
    pType();
    if (pk().type !== 'NAME') { err('期望 NAME'); return; }
    sh();
    rd('param → type NAME', 2);
  }

  function pBlock() {
    if (pk().type !== 'LBRACE') { err('期望 {'); return; }
    sh();
    rd('block → { ...', 1);
    pVarDecls();
    pStmts();
    if (pk().type !== 'RBRACE') { err('期望 }'); return; }
    sh();
    rd('block → ... }', 1);
  }

  function pStmts() {
    let n = 0;
    while (pk().type !== 'RBRACE' && pk().type !== '$') {
      pStmt();
      n++;
    }
    if (n) rd('stmts → stmt+', n);
    else rd('stmts → ε', 0);
  }

  function pStmt() {
    if (pk().type === 'RETURN') {
      sh();
      pExp();
      if (pk().type !== 'SEMICOLON') { err('期望 ;'); return; }
      sh();
      rd('stmt → return exp ;', 3);
      return;
    }
    if (pk().type === 'IF') {
      sh();
      if (pk().type !== 'LPAR') { err('期望 ('); return; }
      sh();
      pExp();
      if (pk().type !== 'RPAR') { err('期望 )'); return; }
      sh();
      pStmt();
      if (pk().type === 'ELSE') {
        sh();
        pStmt();
        rd('stmt → if ( exp ) stmt else stmt', 6);
        return;
      }
      rd('stmt → if ( exp ) stmt', 4);
      return;
    }
    if (pk().type === 'WHILE') {
      sh();
      if (pk().type !== 'LPAR') { err('期望 ('); return; }
      sh();
      pExp();
      if (pk().type !== 'RPAR') { err('期望 )'); return; }
      sh();
      pStmt();
      rd('stmt → while ( exp ) stmt', 5);
      return;
    }
    if (pk().type === 'LBRACE') {
      pBlock();
      rd('stmt → block', 1);
      return;
    }
    pLexp();
    if (pk().type !== 'ASSIGN') { err('期望 ='); return; }
    sh();
    pExp();
    if (pk().type !== 'SEMICOLON') { err('期望 ;'); return; }
    sh();
    rd('stmt → lexp = exp ;', 4);
  }

  function pLexp() {
    pVar();
    while (pk().type === 'LBRACK') {
      sh();
      pExp();
      if (pk().type !== 'RBRACK') { err('期望 ]'); return; }
      sh();
      rd('lexp → lexp [ exp ]', 3);
    }
    if (pk().type === 'DOT') {
      sh();
      if (pk().type !== 'NAME') { err('期望 NAME'); return; }
      sh();
      rd('lexp → lexp . NAME', 3);
    }
    rd('lexp → var', 1);
  }

  const PRECEDENCE = {
    TIMES: 5, DIVIDE: 5, MOD: 5,
    PLUS: 4, MINUS: 4,
    EQUAL: 2, NE: 2, LT: 2, GT: 2, LE: 2, GE: 2,
  };

  function pExpPrec(minPrec) {
    pAtom();
    while (true) {
      const prec = PRECEDENCE[pk().type];
      if (!prec || prec < minPrec) break;
      const op = sh();
      pExpPrec(prec + 1);
      rd('exp → exp ' + op.value + ' exp', 3);
    }
  }

  function pAtom() {
    if (pk().type === 'MINUS') {
      sh();
      pAtom();
      rd('exp → - exp', 2);
      return;
    }
    if (pk().type === 'LPAR') {
      sh();
      pExp();
      if (pk().type !== 'RPAR') { err('期望 )'); return; }
      sh();
      rd('exp → ( exp )', 3);
      return;
    }
    if (pk().type === 'NUMBER') {
      sh();
      rd('exp → NUMBER', 1);
      return;
    }
    if (pk().type === 'FLOAT_LIT') {
      sh();
      rd('exp → FLOAT_LIT', 1);
      return;
    }
    if (pk().type === 'NAME') {
      if (tokens[ti + 1] && tokens[ti + 1].type === 'LPAR') {
        sh(); // NAME
        sh(); // (
        if (pk().type === 'RPAR') {
          sh(); // )
          rd('exp → NAME ( )', 3);
          return;
        }
        pExps();
        if (pk().type !== 'RPAR') { err('期望 )'); return; }
        sh(); // )
        rd('exp → NAME ( exps )', 4);
        return;
      }
      pVar();
      return;
    }
    err('意外 token ' + pk().type);
    sh();
  }

  function pExps() {
    pExp();
    if (pk().type === 'COMMA') {
      sh();
      pExps();
      rd('exps → exp , exps', 3);
    } else {
      rd('exps → exp', 1);
    }
  }

  function pVar() {
    if (pk().type !== 'NAME') { err('期望 NAME'); return; }
    sh();
    rd('var → NAME', 1);
  }

  function pExp() { pExpPrec(0); }

  function pDecl() {
    if (isFunction()) pFunDecl();
    else pVarDecl();
  }

  function pDecls() {
    let n = 0;
    while (pk().type !== '$' && pk().type !== 'RBRACE') {
      pDecl();
      n++;
    }
    if (n) rd('declarations → decl+', n);
    else rd('declarations → ε', 0);
  }

  // Start parsing
  pDecls();
  rd('program → declarations', 1);

  const hasError = events.some(e => e.action === 'error');
  if (!hasError) {
    events.push({ action: 'accept', text: '✓ 解析完成' });
  }

  return { events, hasError };
}

// Build AST tree data from parse events for react-d3-tree
export function buildASTFromEvents(events) {
  const stack = [];

  for (const e of events) {
    if (e.action === 'shift') {
      stack.push({
        name: e.token.value,
        attributes: { type: 'terminal', label: e.token.type },
        _terminal: true,
      });
    } else if (e.action === 'reduce') {
      const arrowIdx = e.text.indexOf('→');
      if (arrowIdx < 0) continue;
      const lhs = e.text.substring(0, arrowIdx).trim();
      const rhsStr = e.text.substring(arrowIdx + 1).trim();
      const n = rhsStr === 'ε' ? 0 : e.symbolCount || 0;

      const children = [];
      for (let j = 0; j < n && stack.length; j++) {
        children.unshift(stack.pop());
      }

      // Only add non-trivial nodes (skip intermediate ε and single-child chains for cleaner tree)
      stack.push({
        name: lhs,
        attributes: { type: 'nonterminal', label: rhsStr },
        children: children.length > 0 ? children : undefined,
      });
    }
  }

  return stack.length > 0 ? stack[stack.length - 1] : null;
}

// Extract symbol table info from tokens
export function extractSymbolTable(tokens) {
  const symbols = [];
  const seen = new Set();

  for (let i = 0; i < tokens.length; i++) {
    const t = tokens[i];
    if (t.type !== 'NAME') continue;

    // Check if this is a declaration (preceded by a type keyword)
    if (i > 0 && ['INT', 'FLOAT', 'STRUCT', 'VOID'].includes(tokens[i - 1].type)) {
      if (!seen.has(t.value)) {
        seen.add(t.value);
        // Determine scope: if inside braces, local; otherwise global
        let scope = 'global';
        let braceCount = 0;
        for (let j = 0; j < i; j++) {
          if (tokens[j].type === 'LBRACE') braceCount++;
          if (tokens[j].type === 'RBRACE') braceCount--;
        }
        if (braceCount > 0) scope = `local (level ${braceCount})`;

        // Determine type
        const typeToken = tokens[i - 1];
        let varType = typeToken.value || typeToken.type;
        // Check if it's a function (next token is '(')
        if (i + 1 < tokens.length && tokens[i + 1].type === 'LPAR') {
          varType = `${varType}()`;
        }

        symbols.push({
          name: t.value,
          type: varType,
          scope,
          line: t.line,
        });
      }
    }
  }

  return symbols;
}
