import { Link } from "react-router";
import {
  FiHome,
  FiArrowLeft,
  FiCpu,
  FiLayers,
  FiGitBranch,
  FiBookOpen,
  FiDatabase,
  FiCode,
} from "react-icons/fi";

const HowItWorks = () => {
  const phases = [
    {
      icon: <FiCode className="w-5 h-5 text-emerald-600" />,
      title: "词法分析",
      desc: "源代码 → Token 流",
      detail: "SeuLex 基于 DFA 最长匹配策略扫描字符流",
    },
    {
      icon: <FiGitBranch className="w-5 h-5 text-emerald-600" />,
      title: "语法分析",
      desc: "LALR(1) 移进-归约",
      detail: "SeuYacc 分析表驱动，自动解决冲突",
    },
    {
      icon: <FiLayers className="w-5 h-5 text-emerald-600" />,
      title: "语法树",
      desc: "归约序列 → AST",
      detail: "每次归约对应一棵子树",
    },
    {
      icon: <FiDatabase className="w-5 h-5 text-emerald-600" />,
      title: "语义分析",
      desc: "符号表 + 类型检查",
      detail: "验证声明、类型匹配、参数校验",
    },
  ];

  const specs = [
    ["SeuLex 规则", "26 条 + 3 定义"],
    ["DFA 状态", "45"],
    ["SeuYacc 产生式", "50 条"],
    ["LR(1) 状态", "200"],
    ["LALR(1) 状态", "95"],
    ["FIRST 集", "19 个"],
  ];

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-50 via-white to-emerald-50 p-4 md:p-6">
      <div className="max-w-4xl mx-auto">
        <header className="flex justify-between items-center mb-8">
          <Link to="/" className="bg-white/90 rounded-lg px-5 py-2 shadow-md border border-emerald-700">
            <span className="text-lg font-semibold text-emerald-800 tracking-wide">
              SeuLex + SeuYacc 可视化
            </span>
          </Link>

          <Link to="/" className="flex items-center gap-2 px-4 py-2 rounded-lg bg-white border border-slate-200 text-slate-600 hover:bg-emerald-50 text-sm">
            <FiArrowLeft className="w-4 h-4" />
            <span>返回</span>
          </Link>
        </header>

        <h2 className="text-2xl font-bold text-slate-800 text-center mb-2">编译流程</h2>
        <p className="text-slate-500 text-center mb-8">从源代码到目标代码的四个阶段</p>

        {/* Phases */}
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4 mb-8">
          {phases.map((p, i) => (
            <div key={i} className="bg-white rounded-xl p-4 border border-emerald-700 shadow-sm">
              <div className="flex items-center gap-3 mb-2">
                <span className="w-6 h-6 rounded-md bg-emerald-600 text-white flex items-center justify-center text-xs font-bold">{i + 1}</span>
                <div>
                  <h3 className="font-semibold text-slate-800">{p.title}</h3>
                  <p className="text-xs text-emerald-700 font-mono">{p.desc}</p>
                </div>
              </div>
              <p className="text-xs text-slate-500 ml-9">{p.detail}</p>
            </div>
          ))}
        </div>

        {/* Specs */}
        <div className="bg-white rounded-xl p-4 border border-emerald-700 shadow-sm mb-8">
          <h3 className="font-semibold text-slate-800 mb-3 flex items-center gap-2">
            <FiCpu className="w-4 h-4 text-emerald-600" /> 技术参数
          </h3>
          <div className="grid grid-cols-2 md:grid-cols-3 gap-2">
            {specs.map(([label, value], i) => (
              <div key={i} className="flex justify-between items-center bg-slate-50 rounded-lg p-2 border border-slate-100 text-xs">
                <span className="text-slate-600">{label}</span>
                <span className="font-medium text-slate-800">{value}</span>
              </div>
            ))}
          </div>
        </div>

        {/* About */}
        <div className="bg-white rounded-xl p-4 border border-emerald-700 shadow-sm mb-8">
          <h3 className="font-semibold text-slate-800 mb-2 flex items-center gap-2">
            <FiBookOpen className="w-4 h-4 text-emerald-600" /> 关于
          </h3>
          <p className="text-sm text-slate-600 leading-relaxed">
            本工具展示 MiniC 编译器前端编译过程。词法分析器由 <strong>SeuLex</strong> 生成（Thompson NFA → DFA → 最小化），
            语法分析器由 <strong>SeuYacc</strong> 生成（LR(1) → LALR(1) 同核合并）。
            东南大学编译原理课程设计作品。
          </p>
        </div>

        <div className="text-center">
          <Link to="/" className="inline-flex items-center gap-2 px-5 py-2 rounded-lg bg-gradient-to-r from-emerald-600 to-green-600 text-white font-medium shadow-sm hover:from-emerald-700 hover:to-green-700 transition-all text-sm">
            <FiHome className="w-4 h-4" /> 试用可视化
          </Link>
        </div>
      </div>
    </div>
  );
};

export default HowItWorks;