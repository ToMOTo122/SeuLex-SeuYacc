import { FiCheckCircle, FiInfo } from "react-icons/fi";

const SymbolTable = ({ symbolTable }) => {
  if (!symbolTable || symbolTable.length === 0) {
    return (
      <div className="bg-emerald-50/50 rounded-lg p-4 text-center border border-emerald-100">
        <p className="text-emerald-700 text-sm">暂无符号信息</p>
        <p className="text-slate-400 text-xs mt-1">带类型声明的变量和函数将显示在此</p>
      </div>
    );
  }

  return (
    <div>
      <div className="mb-3 flex items-center gap-2 bg-emerald-50 p-2 rounded-lg border border-emerald-200">
        <FiCheckCircle className="w-4 h-4 text-emerald-500 flex-shrink-0" />
        <span className="text-xs text-emerald-700">
          符号表构建成功，共 {symbolTable.length} 个符号
        </span>
      </div>

      <div className="overflow-auto rounded-lg border border-slate-200">
        <table className="w-full text-sm">
          <thead className="bg-emerald-50">
            <tr>
              <th className="px-3 py-2 text-left text-xs font-medium text-emerald-800">名称</th>
              <th className="px-3 py-2 text-left text-xs font-medium text-emerald-800">类型</th>
              <th className="px-3 py-2 text-left text-xs font-medium text-emerald-800">作用域</th>
              <th className="px-3 py-2 text-left text-xs font-medium text-emerald-800">行号</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-slate-100">
            {symbolTable.map((symbol, index) => (
              <tr key={index} className="hover:bg-emerald-50/50">
                <td className="px-3 py-2 font-mono text-xs font-medium text-slate-800">{symbol.name}</td>
                <td className="px-3 py-2">
                  <span className="px-2 py-0.5 rounded-full text-xs bg-emerald-100 text-emerald-800 border border-emerald-700">
                    {symbol.type}
                  </span>
                </td>
                <td className="px-3 py-2 text-xs text-slate-500">{symbol.scope}</td>
                <td className="px-3 py-2 text-xs text-slate-500">{symbol.line}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
};

export default SymbolTable;