import { useState, useCallback, useRef, useEffect } from 'react';
import { lexer } from '../services/miniCLexer';
import { parse, buildASTFromEvents, extractSymbolTable } from '../services/miniCParser';

export function useMiniCCompiler() {
  // Compilation results
  const [tokens, setTokens] = useState(null);
  const [parseEvents, setParseEvents] = useState(null);
  const [astTree, setAstTree] = useState(null);
  const [symbolTable, setSymbolTable] = useState(null);
  const [error, setError] = useState(null);
  const [parseError, setParseError] = useState(null);

  // Animation state
  const [phase, setPhase] = useState('idle'); // 'idle' | 'lexing' | 'parsing' | 'done'
  const [charIndex, setCharIndex] = useState(0);
  const [tokenIndex, setTokenIndex] = useState(0);
  const [parseStepIndex, setParseStepIndex] = useState(0);
  const [isPlaying, setIsPlaying] = useState(false);
  const [speed, setSpeed] = useState(5);

  // Internal state (not reactive)
  const sourceCodeRef = useRef('');
  const tokensRef = useRef([]);
  const parseEventsRef = useRef([]);
  const timerRef = useRef(null);

  // Stop any running timer on unmount
  useEffect(() => {
    return () => {
      if (timerRef.current) {
        clearInterval(timerRef.current);
      }
    };
  }, []);

  // Auto-play effect
  useEffect(() => {
    if (!isPlaying || phase === 'idle' || phase === 'done') {
      if (timerRef.current) {
        clearInterval(timerRef.current);
        timerRef.current = null;
      }
      return;
    }

    timerRef.current = setInterval(() => {
      step();
    }, 300 / speed);

    return () => {
      if (timerRef.current) {
        clearInterval(timerRef.current);
        timerRef.current = null;
      }
    };
  }, [isPlaying, phase, speed]);

  // Step function - advances one step in current phase
  const step = useCallback(() => {
    if (phase === 'idle') return;

    if (phase === 'lexing') {
      if (tokenIndex < tokensRef.current.length) {
        const token = tokensRef.current[tokenIndex];
        setCharIndex(token.pos + token.value.length);
        setTokenIndex(prev => prev + 1);
      } else {
        // Lexing done, move to parsing
        setPhase('parsing');
        setCharIndex(sourceCodeRef.current.length);
      }
    } else if (phase === 'parsing') {
      if (parseStepIndex < parseEventsRef.current.length) {
        setParseStepIndex(prev => prev + 1);
      } else {
        // Parsing done
        finishCompilation();
      }
    }
  }, [phase, tokenIndex, parseStepIndex]);

  // Finish compilation - build final results
  const finishCompilation = useCallback(() => {
    setPhase('done');
    setIsPlaying(false);

    const hasError = parseEventsRef.current.some(e => e.action === 'error');
    if (hasError) {
      const errorEvent = parseEventsRef.current.find(e => e.action === 'error');
      setParseError(errorEvent?.text || '语法分析出错');
    } else {
      const ast = buildASTFromEvents(parseEventsRef.current);
      setAstTree(ast);

      const symbols = extractSymbolTable(tokensRef.current);
      setSymbolTable(symbols);
    }
  }, []);

  // Start compilation
  const startCompilation = useCallback((code) => {
    // Reset all state
    setError(null);
    setParseError(null);
    setTokens(null);
    setParseEvents(null);
    setAstTree(null);
    setSymbolTable(null);
    setCharIndex(0);
    setTokenIndex(0);
    setParseStepIndex(0);
    setIsPlaying(false);

    if (timerRef.current) {
      clearInterval(timerRef.current);
      timerRef.current = null;
    }

    sourceCodeRef.current = code;

    try {
      // Run lexer to get all tokens (but don't display all immediately)
      const tokenList = lexer(code);
      tokensRef.current = tokenList;

      // Run parser to get all events (but don't display all immediately)
      const parseResult = parse(tokenList);
      parseEventsRef.current = parseResult.events;

      // Set initial state for animation
      setTokens(tokenList);
      setParseEvents(parseResult.events);
      setPhase('lexing');

    } catch (err) {
      setError(err.message || '编译过程出错');
      setPhase('idle');
    }
  }, []);

  // Reset everything
  const reset = useCallback(() => {
    if (timerRef.current) {
      clearInterval(timerRef.current);
      timerRef.current = null;
    }

    setTokens(null);
    setParseEvents(null);
    setAstTree(null);
    setSymbolTable(null);
    setError(null);
    setParseError(null);
    setPhase('idle');
    setCharIndex(0);
    setTokenIndex(0);
    setParseStepIndex(0);
    setIsPlaying(false);
    sourceCodeRef.current = '';
    tokensRef.current = [];
    parseEventsRef.current = [];
  }, []);

  // Quick compile (skip animation)
  const quickCompile = useCallback((code) => {
    startCompilation(code);
    // Immediately jump to end
    setCharIndex(code.length);
    setTokenIndex(tokensRef.current.length);
    setParseStepIndex(parseEventsRef.current.length);
    finishCompilation();
  }, [startCompilation, finishCompilation]);

  // Play/pause toggle
  const togglePlay = useCallback(() => {
    if (phase === 'idle' || phase === 'done') return;
    setIsPlaying(prev => !prev);
  }, [phase]);

  // Step forward manually
  const stepForward = useCallback(() => {
    if (phase === 'idle' || phase === 'done') return;
    setIsPlaying(false);
    step();
  }, [phase, step]);

  // Get visible tokens (for animation)
  const visibleTokens = tokens ? tokens.slice(0, tokenIndex) : [];

  // Get current parse event (for animation)
  const currentParseEvent = parseEvents ? parseEvents[parseStepIndex - 1] : null;

  return {
    // Results
    tokens,
    parseEvents,
    astTree,
    symbolTable,
    error,
    parseError,

    // Animation state
    phase,
    charIndex,
    tokenIndex,
    parseStepIndex,
    isPlaying,
    speed,
    visibleTokens,
    currentParseEvent,
    sourceCode: sourceCodeRef.current,

    // Actions
    startCompilation,
    quickCompile,
    reset,
    step,
    stepForward,
    togglePlay,
    setSpeed,
  };
}