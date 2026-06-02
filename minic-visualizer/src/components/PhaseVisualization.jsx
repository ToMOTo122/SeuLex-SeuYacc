import LexicalTrace from "./LexicalTrace";
import TokenTable from "./TokenTable";
import ParseTrace from "./ParseTrace";
import ASTVisualization from "./ASTVisualization";
import SymbolTable from "./SymbolTable";
import {
  FiCheckCircle,
  FiDatabase,
  FiInfo,
  FiCode,
  FiLayers,
  FiGitBranch,
  FiZap,
} from "react-icons/fi";

export default function PhaseVisualization({
  tokens,
  parseEvents,
  astTree,
  symbolTable,
  parseError,
  // Animation props
  phase,
  sourceCode,
  charIndex,
  tokenIndex,
  parseStepIndex,
  isPlaying,
  speed,
  visibleTokens,
  currentParseEvent,
  onTogglePlay,
  onStepForward,
  onReset,
  onSpeedChange,
  onSkipLexing,
}) {
  if (!tokens) {
    return (
      <div className="mt-6 p-4 md:p-6 bg-emerald-50 border border-emerald-700 rounded-xl text-emerald-900 flex items-center justify-center">
        <FiInfo className="mr-2 flex-shrink-0" />
        <p>输入代码并点击"编译"查看编译过程。</p>
      </div>
    );
  }

  const PhaseHeader = ({ number, title, icon: Icon, status }) => (
    <h2 className="text-lg font-semibold mb-4 flex items-center gap-2">
      <span className="w-6 h-6 rounded-md bg-emerald-600 text-white flex items-center justify-center text-xs font-bold">
        {number}
      </span>
      <span className="text-slate-700">{title}</span>
      {Icon && <Icon className="text-emerald-600 ml-auto w-4 h-4" />}
      {status && (
        <span className={`text-xs px-2 py-0.5 rounded-full ml-1 ${
          status === 'active' ? 'bg-emerald-100 text-emerald-800' :
          status === 'done' ? 'bg-emerald-100 text-emerald-700' :
          status === 'error' ? 'bg-red-100 text-red-700' :
          'bg-slate-100 text-slate-500'
        }`}>
          {status === 'active' ? '进行中' : status === 'done' ? '完成' : status === 'error' ? '错误' : '等待'}
        </span>
      )}
    </h2>
  );

  // Determine phase statuses
  const lexStatus = phase === 'lexing' ? 'active' : (phase === 'parsing' || phase === 'done') ? 'done' : 'idle';
  const parseStatus = phase === 'parsing' ? 'active' : phase === 'done' ? (parseError ? 'error' : 'done') : 'idle';
  const astStatus = phase === 'done' && !parseError ? 'done' : 'idle';
  const semStatus = phase === 'done' && !parseError ? 'done' : 'idle';

  return (
    <div className="mt-6 space-y-4">
      {/* Phase 1: Lexical Analysis */}
      <div className="bg-white rounded-xl p-4 md:p-6 shadow-sm border border-emerald-700">
        <PhaseHeader
          number="1"
          title="词法分析"
          icon={FiCode}
          status={lexStatus}
        />
        <LexicalTrace
          sourceCode={sourceCode}
          tokens={tokens}
          phase={phase}
          charIndex={charIndex}
          tokenIndex={tokenIndex}
          isPlaying={isPlaying}
          speed={speed}
          onTogglePlay={onTogglePlay}
          onStepForward={onStepForward}
          onReset={onReset}
          onSpeedChange={onSpeedChange}
          onSkipToEnd={onSkipLexing}
        />

        {/* Show full token table after lexing completes */}
        {lexStatus === 'done' && (
          <details className="mt-4">
            <summary className="cursor-pointer text-sm text-emerald-700 font-medium hover:text-emerald-800">
              查看 Token 详细表格 ({tokens.length} tokens)
            </summary>
            <div className="mt-3">
              <TokenTable tokens={tokens} />
            </div>
          </details>
        )}
      </div>

      {/* Phase 2: Syntax Analysis */}
      <div className="bg-white rounded-xl p-4 md:p-6 shadow-sm border border-emerald-700">
        <PhaseHeader
          number="2"
          title="语法分析"
          icon={FiGitBranch}
          status={parseStatus}
        />
        <ParseTrace
          tokens={tokens}
          parseEvents={parseEvents}
          parseError={parseError}
          stepIndex={parseStepIndex}
          isPlaying={isPlaying}
          speed={speed}
          phase={phase}
          onTogglePlay={onTogglePlay}
          onStepForward={onStepForward}
          onReset={onReset}
          onSpeedChange={onSpeedChange}
        />
      </div>

      {/* Phase 3: AST */}
      <div className="bg-white rounded-xl p-4 md:p-6 shadow-sm border border-emerald-700">
        <PhaseHeader
          number="3"
          title="语法树"
          icon={FiLayers}
          status={astStatus}
        />
        <ASTVisualization astTree={astTree} parseError={parseError} />
      </div>

      {/* Phase 4: Semantic Analysis */}
      <div className="bg-white rounded-xl p-4 md:p-6 shadow-sm border border-emerald-700">
        <PhaseHeader
          number="4"
          title="语义分析"
          icon={FiDatabase}
          status={semStatus}
        />
        <SymbolTable symbolTable={symbolTable} />
      </div>
    </div>
  );
}