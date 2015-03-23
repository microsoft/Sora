#pragma once

#include <functional>
#include <vector>

class GridSolutionSolver
{
public:
	GridSolutionSolver();
	bool Solve(double maxValue, double minValue, int height, bool bLog);
	bool SolveWithMaxMinOptimization(double maxBeforeSolve, double minBeforeSolve, double & maxValue, double & minValue, int height, bool bLog);

	void ForEachGridLine(const std::function<void(double dataStep, double dataValue, int pixelValue)> & func);

private:
	struct Solution
	{
		int pixelHeight;
		int gridCount;
		double dataStep;
	};

	struct LineInfo
	{
		double dataValue;
		int pixelValue;
	};

	struct SolutionWithMaxMinOptimization
	{
		double maxValue;
		double minValue;
		double gridRegionCount;
		int gridPixelSize;
		double dataStep;
	};

	static double Calc10BasedNum(double value, int digit);
	static double Ceil(double value1, double value2);
	static double Floor(double value1, double value2);
	bool Solve();
	bool SolveOneLineSolution();
	bool SolveMultiLineSolution(double & maxValue, double & minValue);
	bool SolveWithMaxMinOptimization(double & maxValue, double & minValue);
	bool CalcSolution(double dataStep, Solution & solution);
	void ForEachDataSize(double dataSize, const std::function<void(double dataSize, double dataValue, int pixelValue)> & func);
	bool InPixelRange(int pixel);
	double SolutionValue(double gridCount, double gridSize);

private:
	bool _bLog;
	double _maxValue;
	double _minValue;
	double _maxBeforeSolve;
	double _minBeforeSolve;

	double _minDataStep;
	int _height;
	int _minPixelStep;
	int _topPixelMargin;
	int _bottomPixelMargin;

	double _dataRange;
	bool _bSolved;
	bool _bMaxMinOptimized;


	double _resultDataStep;
	std::vector<LineInfo> _result;
};
