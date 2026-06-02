import { useState } from "react";
import { FiSearch, FiFilter, FiInfo } from "react-icons/fi";
import { tokenColor, tokenCategory } from "../services/miniCLexer";

const TokenTable = ({ tokens }) => {
  const [filterText, setFilterText] = useState("");
  const [showFilter, setShowFilter] = useState(false);

  if (!tokens || tokens.length === 0) {
    return (
      <div className="bg-slate-50 rounded-lg p-4 text-center border border-slate-200">
        <FiInfo className="w-5 h-5 text-slate-400 mx-auto mb-2" />
        <p className="text-slate-500 text-sm">暂无 Token</p>
      </div>
    );
  }

  const normalizedTokens = tokens.map((t, i) => ({
    lexeme: t.value || "",
    token: t.type || "UNKNOWN",
    line: t.line || null,
    _index: i,
  }));

  const filteredTokens = normalizedTokens.filter((t) =>
    t.lexeme.toLowerCase().includes(filterText.toLowerCase()) ||
    t.token.toLowerCase().includes(filterText.toLowerCase())
  );

  const tokenTypes = [...new Set(normalizedTokens.map((t) => t.token))];

  return (
    <div>
      {/* Filter bar */}
      <div className="flex items-center gap-2 mb-2">
        <div className="relative flex-1">
          <FiSearch className="absolute left-2 top-1/2 -translate-y-1/2 w-3 h-3 text-slate-400" />
          <input
            type="text"
            value={filterText}
            onChange={(e) => setFilterText(e.target.value)}
            placeholder="搜索..."
            className="w-full pl-6 pr-2 py-1.5 text-xs rounded-lg border border-slate-200 focus:border-emerald-600 focus:ring-1 focus:ring-emerald-700 outline-none bg-white"
          />
        </div>
        <button
          onClick={() => setShowFilter(!showFilter)}
          className="p-1.5 rounded-lg border border-slate-200 text-slate-500 hover:bg-slate-50"
        >
          <FiFilter className="w-3 h-3" />
        </button>
        <span className="text-xs text-slate-500 px-2">
          {filteredTokens.length}/{tokens.length}
        </span>
      </div>

      {showFilter && (
        <div className="mb-2">
          <select
            value={filterText}
            onChange={(e) => setFilterText(e.target.value)}
            className="w-full px-2 py-1.5 text-xs rounded-lg border border-slate-200 outline-none"
          >
            <option value="">全部类型</option>
            {tokenTypes.map((t) => (
              <option key={t} value={t}>{t}</option>
            ))}
          </select>
        </div>
      )}

      {/* Table */}
      <div className="overflow-auto rounded-lg border border-slate-200 max-h-[200px]">
        <table className="w-full text-xs">
          <thead className="bg-slate-50 sticky top-0">
            <tr>
              <th className="px-2 py-1.5 text-left text-slate-600">#</th>
              <th className="px-2 py-1.5 text-left text-slate-600">值</th>
              <th className="px-2 py-1.5 text-left text-slate-600">类型</th>
              <th className="px-2 py-1.5 text-left text-slate-600">行</th>
              <th className="px-2 py-1.5 text-left text-slate-600">分类</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-slate-100">
            {filteredTokens.map((t, i) => (
              <tr key={t._index} className="hover:bg-emerald-50/50">
                <td className="px-2 py-1 text-slate-400">{t._index + 1}</td>
                <td className="px-2 py-1 font-mono text-slate-700">{t.lexeme}</td>
                <td className="px-2 py-1">
                  <span className={`px-1.5 py-0.5 rounded text-xs ${tokenColor(t.token)}`}>
                    {t.token}
                  </span>
                </td>
                <td className="px-2 py-1 text-slate-500">{t.line}</td>
                <td className="px-2 py-1 text-slate-400">{tokenCategory(t.token)}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
};

export default TokenTable;