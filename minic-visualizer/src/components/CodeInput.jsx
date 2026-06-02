import { useState, useRef } from "react";
import {
  FiCopy,
  FiX,
  FiBookOpen,
  FiChevronDown,
  FiChevronUp,
} from "react-icons/fi";

const CodeInput = ({ code, onChange, onCompile, onReset, isCompiling }) => {
  const [showExamples, setShowExamples] = useState(false);
  const [copied, setCopied] = useState(false);
  const textareaRef = useRef(null);

  const examples = [
    {
      title: "函数调用",
      code: `int a;
int add(int x, int y) {
    return x + y;
}
int main() {
    int x;
    x = add(5, 3);
    if (a == 8)
        return 0;
    else
        return 1;
}`,
    },
    {
      title: "if 嵌套",
      code: `int judge(int a, int b) {
    if (a == b)
        if (a == 10)
            return 1;
        else
            return 2;
    else
        return 0;
}`,
    },
    {
      title: "递归阶乘",
      code: `int fact(int n) {
    if (n == 0)
        return 1;
    else
        return n * fact(n - 1);
}
int main() {
    int r;
    r = fact(5);
    return r;
}`,
    },
    {
      title: "while 循环",
      code: `int sum(int n) {
    int s;
    int i;
    s = 0;
    i = 1;
    while (i <= n) {
        s = s + i;
        i = i + 1;
    }
    return s;
}`,
    },
  ];

  const copyToClipboard = () => {
    navigator.clipboard.writeText(code);
    setCopied(true);
    setTimeout(() => setCopied(false), 1500);
  };

  return (
    <div className="bg-white rounded-xl p-4 shadow-md border border-emerald-700">
      <div className="flex justify-between items-center gap-3 mb-3">
        <div className="flex items-center gap-2">
          <div className="w-7 h-7 bg-gradient-to-br from-emerald-600 to-green-500 rounded-md flex items-center justify-center">
            <FiBookOpen className="w-3 h-3 text-white" />
          </div>
          <span className="text-sm font-semibold text-slate-700">代码输入</span>
        </div>

        <div className="flex gap-2">
          {isCompiling && (
            <button
              onClick={onReset}
              className="px-3 py-1.5 text-xs rounded-lg border border-rose-300 text-rose-600 hover:bg-rose-50 transition-colors font-medium"
            >
              <FiX className="inline mr-1" /> 停止
            </button>
          )}
          <button
            onClick={onCompile}
            disabled={isCompiling || !code.trim()}
            className="px-4 py-1.5 text-xs rounded-lg bg-gradient-to-r from-emerald-600 to-green-500 text-white font-medium shadow-sm hover:from-emerald-700 hover:to-green-600 transition-all disabled:opacity-40 disabled:cursor-not-allowed"
          >
            {isCompiling ? "编译中..." : "编译"}
          </button>
        </div>
      </div>

      <div className="relative group">
        <textarea
          ref={textareaRef}
          value={code}
          onChange={(e) => onChange(e.target.value)}
          className="w-full h-36 p-3 font-mono text-sm rounded-lg border-2 border-slate-200 focus:border-emerald-600 focus:ring-2 focus:ring-emerald-100 outline-none resize-none transition-colors bg-slate-50"
          placeholder="输入 MiniC 代码..."
          spellCheck="false"
        />

        {code && (
          <button
            onClick={copyToClipboard}
            className="absolute top-2 right-2 p-1.5 bg-white/80 rounded-md border border-slate-200 text-slate-400 hover:text-emerald-700 hover:border-emerald-600 transition-colors"
            title="复制"
          >
            <FiCopy className="w-3 h-3" />
          </button>
        )}
      </div>

      {copied && (
        <div className="fixed bottom-4 right-4 bg-emerald-600 text-white px-3 py-2 rounded-lg shadow-lg text-xs z-50">
          ✓ 已复制
        </div>
      )}

      {/* Examples */}
      <div className="mt-3 pt-3 border-t border-slate-100">
        <button
          onClick={() => setShowExamples(!showExamples)}
          className="flex items-center gap-2 text-xs text-slate-500 hover:text-emerald-700 transition-colors"
        >
          <FiBookOpen className="w-3 h-3" />
          <span>示例</span>
          {showExamples ? <FiChevronUp className="w-3 h-3" /> : <FiChevronDown className="w-3 h-3" />}
        </button>

        {showExamples && (
          <div className="grid grid-cols-2 gap-2 mt-2">
            {examples.map((ex, i) => (
              <button
                key={i}
                onClick={() => { onChange(ex.code); setShowExamples(false); }}
                className="text-left p-2 text-xs bg-slate-50 hover:bg-emerald-50 border border-slate-100 hover:border-emerald-700 rounded-lg transition-colors"
              >
                <span className="font-medium text-slate-700">{ex.title}</span>
              </button>
            ))}
          </div>
        )}
      </div>
    </div>
  );
};

export default CodeInput;