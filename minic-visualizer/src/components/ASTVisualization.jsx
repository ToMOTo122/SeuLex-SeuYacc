import { useState, useRef, useEffect } from "react";
import {
  FiAlertTriangle,
  FiCopy,
  FiMaximize,
  FiMinimize,
  FiZoomIn,
  FiZoomOut,
  FiCode,
  FiBookOpen,
  FiLoader,
} from "react-icons/fi";
import Tree from "react-d3-tree";

const ASTVisualization = ({ astTree, parseError }) => {
  const [treeData, setTreeData] = useState(null);
  const [isExpanded, setIsExpanded] = useState(false);
  const [isLoading, setIsLoading] = useState(true);
  const [viewMode, setViewMode] = useState("text");
  const [dimensions, setDimensions] = useState({ width: 800, height: 300 });
  const [zoom, setZoom] = useState(1);
  const treeContainerRef = useRef(null);

  useEffect(() => {
    if (!treeContainerRef.current) return;
    const resizeObserver = new ResizeObserver((entries) => {
      const { width } = entries[0].contentRect;
      setDimensions({ width, height: isExpanded ? 400 : 250 });
    });
    resizeObserver.observe(treeContainerRef.current);
    return () => resizeObserver.disconnect();
  }, [isExpanded]);

  useEffect(() => {
    setIsLoading(true);

    if (parseError) {
      setIsLoading(false);
      return;
    }

    try {
      if (astTree && typeof astTree === "object" && astTree.name) {
        setTreeData(astTree);
      }
      setIsLoading(false);
    } catch (err) {
      setIsLoading(false);
    }
  }, [astTree, parseError]);

  const handleZoomIn = () => setZoom((prev) => Math.min(prev + 0.2, 2));
  const handleZoomOut = () => setZoom((prev) => Math.max(prev - 0.2, 0.5));

  const renderCustomNode = ({ nodeDatum }) => {
    const isTerminal = nodeDatum.attributes?.type === "terminal";
    const bg = isTerminal ? "#fef3c7" : "#d1fae5"; // amber-100 or emerald-100
    const stroke = isTerminal ? "#f59e0b" : "#059669"; // amber-500 or emerald-600
    const textCol = isTerminal ? "#92400e" : "#065f46"; // amber-800 or emerald-800

    return (
      <g>
        <rect
          x="-50"
          y="-20"
          width="100"
          height="40"
          rx="6"
          fill={bg}
          stroke={stroke}
          strokeWidth="1.5"
        />
        <text
          x="0"
          y="2"
          textAnchor="middle"
          fill={textCol}
          style={{ fontSize: "14px", fontFamily: "monospace" }}
        >
          {nodeDatum.name.length > 12 ? nodeDatum.name.slice(0, 10) + "…" : nodeDatum.name}
        </text>
      </g>
    );
  };

  // Text tree renderer
  const renderTextTree = (node, prefix = "", isLast = true) => {
    if (!node) return [];
    const connector = isLast ? "└─" : "├─";
    const lines = [`${prefix}${connector} ${node.name}`];

    if (node.children) {
      const childPrefix = prefix + (isLast ? "   " : "│  ");
      node.children.forEach((child, i) => {
        lines.push(...renderTextTree(child, childPrefix, i === node.children.length - 1));
      });
    }
    return lines;
  };

  if (isLoading) {
    return (
      <div className="flex items-center justify-center py-8 text-slate-500">
        <FiLoader className="w-5 h-5 animate-spin mr-2" />
        <span className="text-sm">生成语法树...</span>
      </div>
    );
  }

  if (parseError) {
    return (
      <div className="flex flex-col items-center justify-center py-8 text-slate-600">
        <FiAlertTriangle className="w-8 h-8 text-rose-400 mb-2" />
        <p className="text-sm">语法错误，无法生成 AST</p>
      </div>
    );
  }

  if (!astTree) {
    return (
      <div className="text-center py-8 text-slate-500 text-sm">
        暂无语法树数据
      </div>
    );
  }

  return (
    <div>
      {/* Controls */}
      <div className="flex gap-2 mb-3">
        <button
          onClick={() => setViewMode("text")}
          className={`px-2 py-1 text-xs rounded-md transition-colors ${
            viewMode === "text"
              ? "bg-emerald-100 text-emerald-800 border border-emerald-700"
              : "bg-slate-50 text-slate-600 border border-slate-200 hover:bg-slate-100"
          }`}
        >
          <FiCode className="inline mr-1 w-3 h-3" /> 文本
        </button>
        <button
          onClick={() => setViewMode("visual")}
          className={`px-2 py-1 text-xs rounded-md transition-colors ${
            viewMode === "visual"
              ? "bg-emerald-100 text-emerald-800 border border-emerald-700"
              : "bg-slate-50 text-slate-600 border border-slate-200 hover:bg-slate-100"
          }`}
        >
          <FiBookOpen className="inline mr-1 w-3 h-3" /> 图形
        </button>
        {viewMode === "visual" && (
          <>
            <button onClick={handleZoomIn} className="px-2 py-1 text-xs bg-slate-50 border border-slate-200 rounded-md">
              <FiZoomIn className="w-3 h-3" />
            </button>
            <button onClick={handleZoomOut} className="px-2 py-1 text-xs bg-slate-50 border border-slate-200 rounded-md">
              <FiZoomOut className="w-3 h-3" />
            </button>
          </>
        )}
        <button
          onClick={() => setIsExpanded(!isExpanded)}
          className="px-2 py-1 text-xs bg-slate-50 border border-slate-200 rounded-md ml-auto"
        >
          {isExpanded ? <FiMinimize className="w-3 h-3" /> : <FiMaximize className="w-3 h-3" />}
        </button>
      </div>

      {/* View */}
      <div
        ref={treeContainerRef}
        className={`bg-slate-50 rounded-lg border border-slate-200 overflow-hidden ${
          isExpanded ? "h-[400px]" : "h-[250px]"
        }`}
      >
        {viewMode === "text" ? (
          <div className="p-3 font-mono text-xs whitespace-pre overflow-auto h-full">
            {renderTextTree(astTree).join("\n")}
          </div>
        ) : (
          <Tree
            data={treeData}
            orientation="vertical"
            renderCustomNodeElement={renderCustomNode}
            translate={{ x: dimensions.width / 2, y: 50 }}
            zoom={zoom}
            pathFunc="step"
            separation={{ siblings: 1.5, nonSiblings: 2 }}
            zoomable
            draggable
            collapsible={false}
            nodeSize={{ x: 120, y: 70 }}
          />
        )}
      </div>
    </div>
  );
};

export default ASTVisualization;