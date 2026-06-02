import { useEffect, useRef } from "react";
import {
  FiPlay,
  FiPause,
  FiSkipForward,
  FiRotateCcw,
  FiZap,
} from "react-icons/fi";
import { tokenColor } from "../services/miniCLexer";

const LexicalTrace = ({
  sourceCode,
  tokens,
  phase,
  charIndex,
  tokenIndex,
  isPlaying,
  speed,
  onTogglePlay,
  onStepForward,
  onReset,
  onSpeedChange,
  onSkipToEnd,
}) => {
  const scanAreaRef = useRef(null);

  // Animation for new tokens
  useEffect(() => {
    // Scroll to latest token if needed
  }, [tokenIndex]);

  if (!sourceCode) {
    return (
      <div className="bg-emerald-50/50 rounded-xl p-8 text-center border border-emerald-100">
        <FiZap className="w-8 h-8 text-emerald-600 mx-auto mb-3" />
        <p className="text-emerald-700 text-sm">点击"编译"开始词法分析</p>
      </div>
    );
  }

  // Render source code with scan highlight
  const renderSourceCode = () => {
    let html = "";
    for (let i = 0; i < sourceCode.length; i++) {
      const c = sourceCode[i];
      const displayChar = c === '\n' ? '↵\n' : c;

      if (i < charIndex) {
        // Already scanned
        html += `<span class="bg-emerald-100 text-emerald-900">${escapeHtml(displayChar)}</span>`;
      } else if (i === charIndex && phase === 'lexing') {
        // Current position - highlight
        html += `<span class="bg-yellow-200 text-yellow-900 border-b-2 border-emerald-600 font-semibold">${escapeHtml(displayChar)}</span>`;
      } else {
        // Not yet scanned
        html += `<span class="text-slate-400">${escapeHtml(displayChar)}</span>`;
      }
    }
    return html;
  };

  // Current token info for DFA display
  const currentToken = tokenIndex > 0 && tokens ? tokens[tokenIndex - 1] : null;

  // Visible tokens for flow
  const visibleTokens = tokens ? tokens.slice(0, tokenIndex) : [];

  return (
    <div className="space-y-4">
      {/* Control Panel */}
      <div className="flex items-center justify-between bg-slate-50 rounded-lg p-2 border border-slate-200">
        <div className="flex gap-1">
          <button
            onClick={onTogglePlay}
            disabled={phase === 'idle' || phase === 'done'}
            className={`px-3 py-1.5 text-xs rounded-lg font-medium transition-all
              ${isPlaying
                ? "bg-emerald-600 text-white shadow-sm"
                : "bg-white border border-emerald-600 text-emerald-800 hover:bg-emerald-50"
              } disabled:opacity-40 disabled:cursor-not-allowed`}
          >
            {isPlaying ? <FiPause className="inline mr-1" /> : <FiPlay className="inline mr-1" />}
            {isPlaying ? "暂停" : "播放"}
          </button>
          <button
            onClick={onStepForward}
            disabled={phase === 'idle' || phase === 'done' || isPlaying}
            className="px-3 py-1.5 text-xs rounded-lg font-medium bg-white border border-slate-200 text-slate-600 hover:bg-slate-50 transition-all disabled:opacity-40 disabled:cursor-not-allowed"
          >
            <FiSkipForward className="inline mr-1" /> 单步
          </button>
          <button
            onClick={onReset}
            className="px-3 py-1.5 text-xs rounded-lg font-medium bg-white border border-slate-200 text-slate-600 hover:bg-slate-50 transition-all"
          >
            <FiRotateCcw className="inline mr-1" /> 重置
          </button>
          {phase === 'lexing' && (
            <button
              onClick={onSkipToEnd}
              className="px-3 py-1.5 text-xs rounded-lg font-medium bg-white border border-emerald-700 text-emerald-700 hover:bg-emerald-50 transition-all"
            >
              <FiZap className="inline mr-1" /> 快速
            </button>
          )}
        </div>

        <div className="flex items-center gap-3">
          <span className="text-xs text-slate-500">
            {phase === 'lexing' && `词法: ${tokenIndex}/${tokens?.length || 0} tokens`}
            {phase === 'parsing' && "词法完成 ✓"}
            {phase === 'done' && "完成 ✓"}
            {phase === 'idle' && "就绪"}
          </span>
          <div className="flex items-center gap-1">
            <span className="text-xs text-slate-400">速度</span>
            <input
              type="range"
              min="1"
              max="10"
              value={speed}
              onChange={(e) => onSpeedChange(parseInt(e.target.value))}
              className="w-16 h-1 accent-emerald-600"
            />
            <span className="text-xs text-slate-500 w-4">{speed}</span>
          </div>
        </div>
      </div>

      {/* Source Code Scan Area */}
      <div className="relative">
        <div className="text-xs text-slate-500 mb-1 font-medium">源代码扫描</div>
        <div
          ref={scanAreaRef}
          className="bg-white rounded-lg border-2 border-emerald-700 p-3 font-mono text-sm leading-relaxed whitespace-pre-wrap overflow-auto max-h-[150px] shadow-inner"
          dangerouslySetInnerHTML={{ __html: renderSourceCode() }}
        />
      </div>

      {/* DFA State Transition */}
      {currentToken && phase !== 'idle' && (
        <div className="bg-gradient-to-r from-emerald-50 to-green-50 rounded-lg p-2 border border-emerald-700 font-mono text-xs">
          <span className="text-slate-500">DFA: </span>
          <span className="text-emerald-800 font-medium">S0</span>
          <span className="text-slate-400"> ─</span>
          <span className="text-emerald-700 font-semibold">'{currentToken.value}'</span>
          <span className="text-slate-400">→ </span>
          <span className="text-emerald-600">✓</span>
          <span className={`ml-1 px-2 py-0.5 rounded ${tokenColor(currentToken.type)}`}>
            {currentToken.type}
          </span>
        </div>
      )}

      {/* Token Flow */}
      <div>
        <div className="text-xs text-slate-500 mb-1 font-medium">Token 流</div>
        <div className="bg-white rounded-lg border border-slate-200 p-2 min-h-[40px] flex flex-wrap gap-1 content-start">
          {visibleTokens.length === 0 ? (
            <span className="text-slate-400 text-xs italic">等待识别...</span>
          ) : (
            visibleTokens.map((token, i) => (
              <span
                key={i}
                className={`text-xs px-2 py-1 rounded animate-fadeIn ${tokenColor(token.type)}`}
              >
                {token.type} <span className="opacity-60">'{token.value}'</span>
              </span>
            ))
          )}
        </div>
      </div>
    </div>
  );
};

function escapeHtml(s) {
  return String(s)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/'/g, '&#39;');
}

export default LexicalTrace;