import { useEffect, useRef } from "react";
import {
  FiPlay,
  FiPause,
  FiSkipForward,
  FiRotateCcw,
  FiInfo,
} from "react-icons/fi";
import { tokenDisplay } from "../services/miniCLexer";

const ParseTrace = ({
  tokens,
  parseEvents,
  parseError,
  // New props from hook
  stepIndex,
  isPlaying,
  speed,
  phase,
  onTogglePlay,
  onStepForward,
  onReset,
  onSpeedChange,
}) => {
  const scrollRef = useRef(null);

  if (!parseEvents || parseEvents.length === 0) {
    return (
      <div className="bg-emerald-50/50 rounded-xl p-8 text-center border border-emerald-100">
        <FiInfo className="w-8 h-8 text-emerald-600 mx-auto mb-3" />
        <p className="text-emerald-700 text-sm">词法分析完成后开始语法分析</p>
      </div>
    );
  }

  // Reconstruct stack and consumed tokens up to current step
  const reconstructState = () => {
    const stack = [];
    let consumedCount = 0;

    for (let i = 0; i < stepIndex && i < parseEvents.length; i++) {
      const e = parseEvents[i];
      if (e.action === "shift") {
        stack.push(tokenDisplay(e.token));
        consumedCount = e.tokenIndex + 1;
      } else if (e.action === "reduce") {
        const arrowIdx = e.text.indexOf("→");
        if (arrowIdx > 0) {
          const rhsStr = e.text.substring(arrowIdx + 1).trim();
          const n = rhsStr === "ε" ? 0 : e.symbolCount || 0;
          for (let j = 0; j < n && stack.length; j++) stack.pop();
          stack.push(e.text.substring(0, arrowIdx).trim());
        }
      }
    }

    return { stack, consumedCount };
  };

  const { stack, consumedCount } = reconstructState();
  const currentEvent = stepIndex > 0 ? parseEvents[stepIndex - 1] : null;

  const getActionStyle = (action) => {
    switch (action) {
      case "shift":
        return "bg-green-50 text-cyan-800 border-l-3 border-green-600";
      case "reduce":
        return "bg-emerald-50 text-emerald-800 border-l-3 border-emerald-500";
      case "accept":
        return "bg-emerald-50 text-emerald-900 border-l-3 border-emerald-600 font-semibold";
      case "error":
        return "bg-rose-50 text-rose-800 border-l-3 border-rose-500";
      default:
        return "bg-slate-50 text-slate-800";
    }
  };

  const getActionText = (event) => {
    switch (event.action) {
      case "shift":
        return `移进: ${tokenDisplay(event.token)}`;
      case "reduce":
        return `归约: ${event.text}`;
      case "accept":
        return event.text;
      case "error":
        return `错误: ${event.text}`;
      default:
        return "";
    }
  };

  return (
    <div>
      {/* Controls */}
      <div className="flex items-center justify-between mb-3 bg-slate-50 rounded-lg p-2 border border-slate-200">
        <div className="flex gap-1">
          <button
            onClick={onTogglePlay}
            disabled={phase !== 'parsing'}
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
            disabled={phase !== 'parsing' || isPlaying || stepIndex >= parseEvents.length}
            className="px-3 py-1.5 text-xs rounded-lg font-medium bg-white border border-slate-200 text-slate-600 hover:bg-slate-50 transition-all disabled:opacity-40 disabled:cursor-not-allowed"
          >
            <FiSkipForward className="inline mr-1" /> 单步
          </button>
        </div>

        <div className="flex items-center gap-2">
          <span className="text-xs text-slate-500">
            步骤 {stepIndex}/{parseEvents.length}
            {parseError && <span className="text-rose-500 ml-1">· {parseError}</span>}
          </span>
          <input
            type="range"
            min="1"
            max="10"
            value={speed}
            onChange={(e) => onSpeedChange(parseInt(e.target.value))}
            className="w-14 h-1 accent-emerald-600"
          />
        </div>
      </div>

      {/* Stack and Input */}
      <div className="grid grid-cols-2 gap-3 mb-3">
        <div>
          <div className="text-xs text-slate-500 mb-1 font-medium">栈</div>
          <div className="bg-slate-50 rounded-lg border border-slate-200 p-2 min-h-[80px] font-mono text-xs overflow-auto">
            {stack.length === 0 ? (
              <span className="text-slate-400 italic">空</span>
            ) : (
              stack.map((item, i) => (
                <div
                  key={i}
                  className={`py-0.5 pl-2 border-l-2 ${
                    i === stack.length - 1
                      ? "border-emerald-600 bg-emerald-50 text-emerald-800 font-medium"
                      : "border-slate-200 text-slate-600"
                  }`}
                >
                  {item}
                </div>
              ))
            )}
          </div>
        </div>

        <div>
          <div className="text-xs text-slate-500 mb-1 font-medium">剩余输入</div>
          <div className="bg-slate-50 rounded-lg border border-slate-200 p-2 min-h-[80px] font-mono text-xs overflow-auto">
            {tokens.slice(consumedCount).map((t, i) => (
              <span
                key={i}
                className={`inline-block px-1 ${
                  i === 0 ? "bg-yellow-200 rounded text-yellow-800 font-medium" : "text-slate-600"
                }`}
              >
                {tokenDisplay(t)}
              </span>
            ))}
          </div>
        </div>
      </div>

      {/* Current Action */}
      {currentEvent && (
        <div className={`p-2.5 rounded-lg font-mono text-xs mb-3 ${getActionStyle(currentEvent.action)}`}>
          {getActionText(currentEvent)}
        </div>
      )}

      {/* Trace Log */}
      <details className="bg-slate-50 rounded-lg border border-slate-200">
        <summary className="cursor-pointer p-3 text-xs font-medium text-emerald-700 hover:bg-slate-100 rounded-lg">
          完整轨迹 ({parseEvents.length} 步)
        </summary>
        <div ref={scrollRef} className="p-3 border-t border-slate-200 max-h-[150px] overflow-auto">
          <table className="w-full text-xs font-mono">
            <tbody>
              {parseEvents.map((e, i) => (
                <tr
                  key={i}
                  className={`border-b border-slate-100 ${i === stepIndex - 1 ? "bg-emerald-50" : ""}`}
                >
                  <td className="py-1 px-2 text-slate-400 w-8">{i + 1}</td>
                  <td className="py-1 px-2 w-16">
                    <span className={`px-1 py-0.5 rounded text-xs ${
                      e.action === "shift" ? "bg-green-100 text-green-800" :
                      e.action === "reduce" ? "bg-emerald-100 text-emerald-700" :
                      e.action === "accept" ? "bg-emerald-100 text-emerald-800" :
                      "bg-rose-100 text-rose-700"
                    }`}>
                      {e.action === "shift" ? "移进" : e.action === "reduce" ? "归约" : e.action === "accept" ? "接受" : "错误"}
                    </span>
                  </td>
                  <td className="py-1 px-2 text-slate-700">
                    {e.action === "shift" ? tokenDisplay(e.token) : e.text}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </details>

      {/* Explanation */}
      <div className="mt-3 flex items-start gap-2 bg-green-50 p-2 rounded-lg border border-green-100">
        <FiInfo className="text-green-600 w-4 h-4 mt-0.5 flex-shrink-0" />
        <div className="text-xs text-green-800">
          <span className="font-medium">移进-归约分析</span> —
          移进: 压入输入符号; 归约: 用产生式左部替换栈顶; 接受: 分析成功
        </div>
      </div>
    </div>
  );
};

export default ParseTrace;