//
// Copyright (C) 2013-2018 University of Amsterdam
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef ENGINE_H
#define ENGINE_H

#include "enginedefinitions.h"
#include "dataset.h"
#include "ipcchannel.h"
#include <json/json.h>
#include "columnencoder.h"

/// The Engine handles communication between Desktop and R
/// It can be in a variety of states _currentEngineState and can run analyses, filters, compute columns and Rcode.
/// It also contains some utility functions for use by rbridge and by extension R
class Engine
{
public:
	typedef engineAnalysisStatus Status;
	
	explicit				Engine(int slaveNo, unsigned long parentPID);
							~Engine();
	static Engine		*	theEngine() { return _EngineInstance; } //There is only ever one engine in a process so we might as well have a static pointer to it.

	void					run();
	bool					receiveMessages(int timeout = 0);
	void					setSlaveNo(int no);
	int						engineNum() const { return _engineNum; }
	void					sendString(std::string message);

	

	Status					getAnalysisStatus() { return _analysisStatus; }
	analysisResultStatus	getStatusToAnalysisStatus();

	//the following functions in public can all be called (indirectly) from R and/or rbridge:
	int						getColumnType(			const std::string & columnName);
	int						getColumnAnalysisId(	const std::string & columnName);
	std::string				createColumn(			const std::string & columnName); ///< Returns encoded columnname on success or "" on failure (cause it already exists)
	bool					deleteColumn(			const std::string & columnName);
	bool					setColumnDataAndType(	const std::string & columnName, const	std::vector<std::string>	& nominalData, columnType colType); ///< return true for any changes
	bool					isColumnNameOk(			const std::string & columnName);
	int						dataSetRowCount()		{ return static_cast<int>(provideAndUpdateDataSet()->rowCount()); }
	bool					paused()				{ return _engineState == engineState::paused; }
	DataSet				*	provideAndUpdateDataSet();

	void					provideTempFileName(		const std::string & extension,		std::string & root,	std::string & relativePath);
	void					provideStateFileName(											std::string & root,	std::string & relativePath);
	void					provideJaspResultsFileName(									std::string & root,	std::string & relativePath);
	void					provideSpecificFileName(	const std::string & specificName,	std::string & root,	std::string & relativePath);
	void					reloadColumnNames();

private:
	void					initialize();
	void					beIdle(bool newlyIdle);

	void					receiveRCodeMessage(			const Json::Value & jsonRequest);
	void					receiveFilterMessage(			const Json::Value & jsonRequest);
	void					receiveAnalysisMessage(			const Json::Value & jsonRequest);
	void					receiveComputeColumnMessage(	const Json::Value & jsonRequest);
	void					receiveModuleRequestMessage(	const Json::Value & jsonRequest);
	void					receiveReloadData();
	void					receiveLogCfg(					const Json::Value & jsonRequest);
	void					receiveSettings(				const Json::Value & jsonRequest);
	void					absorbSettings(					const Json::Value & json);

	void					runAnalysis();
	void					runComputeColumn(	const std::string & computeColumnName,	const std::string & computeColumnCode,	columnType computeColumnType,	bool forceType);
	void					runFilter(			const std::string & filter,				const std::string & generatedFilter,	int filterRequestId				);
	void					runRCode(			const std::string & rCode,				int rCodeRequestId,						bool whiteListed				);
	void					runRCodeCommander(		  std::string   rCode																						);

	void					stopEngine();
	void					pauseEngine(	const Json::Value & jsonRequest);
	void					resumeEngine(	const Json::Value & jsonRequest); 
	void					sendEnginePaused();
	void					sendEngineResumed(bool justReloadedData = false);
	void					sendEngineLoadingData();
	void					sendEngineStopped();

	void					saveImage();
	void					editImage();
	void					rewriteImages();
	void					removeNonKeepFiles(const Json::Value & filesToKeepValue);

	void					sendAnalysisResults();
	void					sendFilterResult(		int filterRequestId);
	void					sendFilterError(		int filterRequestId,	const std::string & errorMessage);
	void					sendRCodeResult(		int rCodeRequestId,		const std::string & rCodeResult);
	void					sendRCodeError(			int rCodeRequestId);

public:


private: // Data:
	static Engine		*	_EngineInstance;
	const int				_engineNum;
	const unsigned long		_parentPID;
	DataSet				*	_dataSet				= nullptr;
	DatabaseInterface	*	_db						= nullptr;
	IPCChannel			*	_channel				= nullptr;
	ColumnEncoder		*	_extraEncodings			= nullptr;
	engineState				_engineState			= engineState::initializing,
							_lastRequest			= engineState::initializing;
	Status					_analysisStatus			= Status::empty;
	int						_analysisId,
							_analysisRevision,
							_progress,
							_ppi					= 96,
							_numDecimals			= 3;
	bool					_developerMode			= false,
							_fixedDecimals			= false,
							_exactPValues			= false,
							_normalizedNotation		= true;
	std::string				_analysisName,
							_analysisTitle,
							_analysisDataKey,
							_analysisResultsMeta,
							_analysisStateKey,
							_analysisResultsString,
							_resultFont,
							_imageBackground		= "white",
							_analysisRFile			= "",
							_dynamicModuleCall		= "",
							_langR					= "en";
	Json::Value				_imageOptions,
							_analysisOptions		= Json::nullValue,
							_analysisResults;


};

#endif // ENGINE_H
