import { useState } from "react";
import { BrowserRouter as Router, Routes, Route, Link } from "react-router";
import {
  FiZap,
  FiAlertTriangle,
  FiHome,
  FiBookOpen,
} from "react-icons/fi";
import CodeInput from "./components/CodeInput";
import PhaseVisualization from "./components/PhaseVisualization";
import HowItWorks from "./components/HowItWorks";
import Footer from "./components/Footer";
import { useMiniCCompiler } from "./hooks/useMiniCCompiler";

const CompilerVisualizer = () => {
  const [code, setCode] = useState(`int main() {
    int x;
    x = 10 + 5;
    if (x == 15)
        return 1;
    else
        return 0;
}`);

  const {
    tokens,
    parseEvents,
    astTree,
    symbolTable,
    error,
    parseError,
    phase,
    sourceCode,
    charIndex,
    tokenIndex,
    parseStepIndex,
    isPlaying,
    speed,
    visibleTokens,
    currentParseEvent,
    startCompilation,
    reset,
    stepForward,
    togglePlay,
    setSpeed,
  } = useMiniCCompiler();

  const handleCompile = () => {
    startCompilation(code);
  };

  const handleReset = () => {
    reset();
  };

  const handleSkipLexing = () => {
    // Fast forward through lexing
    while (phase === 'lexing') {
      stepForward();
    }
  };

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-50 via-white to-emerald-50 p-3 md:p-6">
      <div className="max-w-5xl mx-auto">
        {/* Header - 简洁文字标题 */}
        <header className="flex flex-col md:flex-row justify-between items-center mb-6 gap-3">
          <Link
            to="/"
            className="bg-white/90 rounded-lg px-5 py-2 shadow-md border border-emerald-700 hover:shadow-lg transition-shadow"
          >
            <span className="text-lg font-semibold text-emerald-800 tracking-wide">
              SeuLex + SeuYacc 可视化
            </span>
          </Link>

          <nav className="flex gap-2">
            <Link
              to="/"
              className="flex items-center gap-2 px-3 py-1.5 rounded-lg bg-white border border-slate-200 text-slate-600 hover:bg-emerald-50 hover:border-emerald-600 transition-colors text-sm"
            >
              <FiHome className="w-4 h-4" />
              <span>主页</span>
            </Link>
            <Link
              to="/how-it-works"
              className="flex items-center gap-2 px-3 py-1.5 rounded-lg bg-white border border-slate-200 text-slate-600 hover:bg-emerald-50 hover:border-emerald-600 transition-colors text-sm"
            >
              <FiBookOpen className="w-4 h-4" />
              <span>原理</span>
            </Link>
          </nav>
        </header>

        {/* Quick Start Guide */}
        {phase === 'idle' && (
          <div className="bg-emerald-50 rounded-lg p-3 border border-emerald-600 mb-5">
            <div className="flex items-center gap-2 mb-2">
              <FiZap className="w-4 h-4 text-emerald-700" />
              <span className="text-sm font-medium text-emerald-800">快速指南</span>
            </div>
            <div className="grid grid-cols-1 md:grid-cols-3 gap-3 text-xs text-slate-600">
              <div className="flex items-center gap-2">
                <span className="w-5 h-5 rounded bg-emerald-100 text-emerald-800 flex items-center justify-center text-xs font-bold border border-emerald-300">1</span>
                <span>输入 MiniC 代码</span>
              </div>
              <div className="flex items-center gap-2">
                <span className="w-5 h-5 rounded bg-emerald-100 text-emerald-800 flex items-center justify-center text-xs font-bold border border-emerald-300">2</span>
                <span>点击编译按钮</span>
              </div>
              <div className="flex items-center gap-2">
                <span className="w-5 h-5 rounded bg-emerald-100 text-emerald-800 flex items-center justify-center text-xs font-bold border border-emerald-300">3</span>
                <span>观察编译动画</span>
              </div>
            </div>
          </div>
        )}

        {/* Code Input */}
        <CodeInput
          code={code}
          onChange={setCode}
          onCompile={handleCompile}
          onReset={handleReset}
          isCompiling={phase !== 'idle'}
        />

        {/* Error Display */}
        {error && (
          <div className="mt-4 flex items-center gap-2 bg-red-50 border border-red-200 text-red-700 px-4 py-3 rounded-lg">
            <FiAlertTriangle className="w-5 h-5 flex-shrink-0" />
            <span className="text-sm">{error}</span>
          </div>
        )}

        {/* Compilation Phases */}
        {tokens && (
          <PhaseVisualization
            tokens={tokens}
            parseEvents={parseEvents}
            astTree={astTree}
            symbolTable={symbolTable}
            parseError={parseError}
            // Animation props
            phase={phase}
            sourceCode={sourceCode}
            charIndex={charIndex}
            tokenIndex={tokenIndex}
            parseStepIndex={parseStepIndex}
            isPlaying={isPlaying}
            speed={speed}
            visibleTokens={visibleTokens}
            currentParseEvent={currentParseEvent}
            onTogglePlay={togglePlay}
            onStepForward={stepForward}
            onReset={handleReset}
            onSpeedChange={setSpeed}
            onSkipLexing={handleSkipLexing}
          />
        )}
      </div>
    </div>
  );
};

const App = () => {
  return (
    <Router>
      <Routes>
        <Route path="/" element={<CompilerVisualizer />} />
        <Route path="/how-it-works" element={<HowItWorks />} />
      </Routes>
      <Footer />
    </Router>
  );
};

export default App;