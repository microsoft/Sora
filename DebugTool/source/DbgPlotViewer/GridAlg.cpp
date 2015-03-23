#include "stdafx.h"
#include <assert.h>
#include <algorithm>
#include "GridAlg.h"

using namespace std;

#define MAX_SOLUTION 128

GridSolutionSolver::GridSolutionSolver()
{
	_minDataStep = 0.00001;
	_minPixelStep = 20;
	_topPixelMargin = 18;
	_bottomPixelMargin = 15;

	_maxValue = _maxBeforeSolve = 10.0;
	_minValue = _minBeforeSolve = 0.0;

	_bSolved = false;
	_bMaxMinOptimized = false;
}

double GridSolutionSolver::Calc10BasedNum(double value, int digit)
{
	assert(value > 0);
	return pow(10.0, floor(log10(value)-digit));
}

double GridSolutionSolver::Ceil(double value1, double value2)
{
	assert(value2 > 0.0);
	return ceil(value1 / value2) * value2;
}

double GridSolutionSolver::Floor(double value1, double value2)
{
	assert(value2 > 0.0);
	return floor(value1 / value2) * value2;
}

bool GridSolutionSolver::InPixelRange(int pixel)
{
	return (pixel <= (_height - _topPixelMargin)) && (pixel >= _bottomPixelMargin);
}

void GridSolutionSolver::ForEachDataSize(double dataSize, const std::function<void(double dataStep, double dataValue, int pixelValue)> & func)
{
	double current = Floor(_minValue, dataSize);
	double upBound = Ceil(_maxValue, dataSize);

	while(current <= upBound)
	{
		int line =  (int)(_height / _dataRange * (current - _minValue));
		if (InPixelRange(line))
			func(dataSize, current, line);

		current += dataSize;
	}
}

bool GridSolutionSolver::CalcSolution(double dataStep, Solution & solution)
{
	int gridCount = 0;
	int pixelHeight = (int)(_height / _dataRange * dataStep);

	if (pixelHeight < _minPixelStep)
		return false;

	ForEachDataSize(dataStep, [&](double dataStep, double dataValue, int pixelValue){
		gridCount++;
	});

	if (gridCount > 0 )
	{
		solution.gridCount = gridCount;
		solution.pixelHeight = pixelHeight;
		solution.dataStep = dataStep;
		return true;
	}

	return false;
}

bool GridSolutionSolver::Solve(double maxValue, double minValue, int height, bool bLog)
{
	if ( _bSolved && (_maxValue == maxValue) && (_minValue == minValue) && (_height == height) && (bLog == bLog) )
	{
		return true;
	}
	else
	{
		_bMaxMinOptimized = false;
		_maxValue = maxValue;
		_minValue = minValue;
		_height = height;
		_bLog = bLog;

		_bSolved = Solve();

		return _bSolved;
	}
}

bool GridSolutionSolver::SolveWithMaxMinOptimization(double maxBeforeSolve, double minBeforeSolve, double & maxValue, double & minValue, int height, bool bLog)
{
	if (_bMaxMinOptimized && (_maxBeforeSolve == maxBeforeSolve) && (_minBeforeSolve == minBeforeSolve) && (_height == height) && (bLog == bLog))
	{
		maxValue = _maxValue;
		minValue = _minValue;
		return true;
	}
	else
	{
		_height = height;
		_bLog = bLog;
		maxValue = _maxBeforeSolve = maxBeforeSolve;
		minValue = _minBeforeSolve = minBeforeSolve;

		_bSolved = SolveWithMaxMinOptimization(maxValue, minValue);
		_bMaxMinOptimized = _bSolved;
		return _bSolved;
	}
}

bool GridSolutionSolver::Solve()
{
	_dataRange = _maxValue - _minValue;

	double baseDataStep = Calc10BasedNum(_dataRange, 1);

	Solution solution;
	Solution solutions[MAX_SOLUTION];
	int currentSolutionIdx = 0;

	double candidates[] = {1.0, 2.0, 5.0, 10.0, 20.0, 50.0};
	
	for (int i = 0; i < sizeof(candidates)/sizeof(*candidates); ++i)
	{
		double candidate = candidates[i];
		double dataStep = baseDataStep * candidate;
		if (CalcSolution(dataStep, solution))
		{
			solutions[currentSolutionIdx++] = solution;
			if (currentSolutionIdx == MAX_SOLUTION)
				break;
		}
	}

	Solution bestSolution;
	double minMark;
	bool bSolutionFound = false;

	if (currentSolutionIdx > 0)
	{
		for (int i = 0; i < currentSolutionIdx; ++i)
		{
			if (!bSolutionFound)
			{
				bSolutionFound = true;
				bestSolution = solutions[i];
				minMark = SolutionValue(bestSolution.gridCount, bestSolution.pixelHeight);
			}
			else
			{
				Solution curSolution = solutions[i];
				double curMark = SolutionValue(solution.gridCount, solution.pixelHeight);
				if (curMark < minMark)
				{
					bestSolution = curSolution;
					minMark = curMark;
				}
			}
		}

		_resultDataStep = bestSolution.dataStep;

		_result.clear();
		ForEachDataSize(_resultDataStep, [this](double dataStep, double dataValue, int pixelValue){
			LineInfo lineInfo;
			lineInfo.dataValue = dataValue;
			lineInfo.pixelValue = pixelValue;
			_result.push_back(lineInfo);
		});

		return true;
	}

	return false;
}

bool GridSolutionSolver::SolveWithMaxMinOptimization(double & maxValue, double & minValue)
{
	if ( _height < (_topPixelMargin + _bottomPixelMargin) )		// an empty solution
	{
		_result.clear();
		_resultDataStep = 0.0;
		return true;	// an empty solution
	}

	if (_height < (_minPixelStep * 2 + _topPixelMargin + _bottomPixelMargin))	// an one line solution
	{
		return SolveOneLineSolution();
	}

	return SolveMultiLineSolution(maxValue, minValue);		// multiline solution
}

bool GridSolutionSolver::SolveOneLineSolution()
{
	LineInfo lineInfo;
	lineInfo.dataValue = (_maxValue + _minValue) / 2;
	lineInfo.pixelValue = _height / 2;

	_result.clear();
	_result.push_back(lineInfo);

	return true;
}

bool GridSolutionSolver::SolveMultiLineSolution(double & maxValue, double & minValue)
{
	double dataRange = maxValue - minValue;

	int gridRegionCount = 3;

	SolutionWithMaxMinOptimization solutions[MAX_SOLUTION];
	int curSolutionIdx = 0;

	int lastGridRegionCount = -1;
	double lastdataStepBaseAfterRound = -1.0;

	while(1)
	{
		int gridPixelSize = _height / gridRegionCount;
		if (gridPixelSize < _minPixelStep)
			break;

		double baseDataStep = dataRange * gridPixelSize / _height;
		double dataStepBaseAfterRound = pow(10.0, ceil(log10(baseDataStep)));
		if (dataStepBaseAfterRound == lastdataStepBaseAfterRound)
		{
			gridRegionCount++;
			continue;
		}

		double factors[] = {1.0, 0.5, 0.2};

		for (int iCandidate = 0; iCandidate < sizeof(factors)/sizeof(*factors); ++iCandidate)
		{
			double dataStepAfterRound = dataStepBaseAfterRound * factors[iCandidate];
			double maxValueRounded = Ceil(maxValue, dataStepAfterRound);
			double minValueRounded = Floor(minValue, dataStepAfterRound);

			int actualGridRegionCount = (int)(((maxValueRounded - minValueRounded) / dataStepAfterRound) + 0.5);

			int pixelHeight = _height / actualGridRegionCount;
			if (pixelHeight < _minPixelStep)
			{
				gridRegionCount++;
				continue;
			}

			SolutionWithMaxMinOptimization solution;
			solution.maxValue = maxValueRounded;
			solution.minValue = minValueRounded;
			solution.gridRegionCount = actualGridRegionCount;
			solution.dataStep = dataStepAfterRound;
			solution.gridPixelSize = pixelHeight;

			solutions[curSolutionIdx++] = solution;
			if (curSolutionIdx == MAX_SOLUTION)
				break;
		}

		if (curSolutionIdx == MAX_SOLUTION)
			break;

		gridRegionCount++;
	}

	SolutionWithMaxMinOptimization bestSolution;
	double mark = 100000.0;
	bool bSolutionFound = false;

	for_each(solutions, solutions + curSolutionIdx, [&](const SolutionWithMaxMinOptimization & solution){
		if (!bSolutionFound)
		{
			bSolutionFound = true;
			bestSolution = solution;
			mark = SolutionValue(solution.gridRegionCount - 1, solution.gridPixelSize);
			return;
		}

		double thisMark = SolutionValue(solution.gridRegionCount - 1, solution.gridPixelSize);
		if (thisMark < mark)
		{
			bestSolution = solution;
			mark = thisMark;
		}
	});

	if (bSolutionFound)
	{
		maxValue = _maxValue = bestSolution.maxValue;
		minValue = _minValue = bestSolution.minValue;

		_dataRange = _maxValue - _minValue;
		_resultDataStep = bestSolution.dataStep;

		_result.clear();
		ForEachDataSize(bestSolution.dataStep, [this](double dataSize, double dataValue, int pixelValue){
			LineInfo lineInfo;
			lineInfo.dataValue = dataValue;
			lineInfo.pixelValue = pixelValue;
			this->_result.push_back(lineInfo);
		});

		return true;
	}

	return false;
}

void GridSolutionSolver::ForEachGridLine(const std::function<void(double dataStep, double dataValue, int pixelValue)> & func)
{
	if (!_bSolved)
		return;

	for_each(_result.begin(), _result.end(), [this, func](const LineInfo & info){
		func(_resultDataStep, info.dataValue, info.pixelValue);
	});
}

double GridSolutionSolver::SolutionValue(double gridCount, double gridSize)
{
	double value = abs(gridCount - 7) + abs(gridSize - 35);
	if (gridCount == 1)
		value += 10000;	// 1 is very bad
	return value;
}
