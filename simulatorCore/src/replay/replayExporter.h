/**
 * @file   replayExporter.h
 * @author Pierre Thalamy <pthalamy@pierre-ZenBook-UX433FA>
 * @date   Tue Jun  9 11:13:33 2020
 *
 * @brief  Simulation data exporter to be used for simulation replay
 *
 *
 */

#pragma once

#include <fstream>

#include "utils/exceptions.h"

using namespace std;

/**
 * Configuration exporter that outputs all relevant simulation data to an export file
 *  for simulation reconstruction using a player.
 * Export file will be name replay_<appName>_<confName>_timestamp.vs
 * @note To be used as a singleton instance
 */
class ReplayExporter {
    static inline ReplayExporter* singleton = nullptr; //!< the singleton instance

    static const bool debug = true; //!< Indicates whether to write the debug file or not
    static inline const string extension = "vs"; //!< Export file extension

    /**
     * The frequency of key frame export in MICROSECONDS.
     * Saves a key frame every <N> MICROSECONDS (us)
     * @attention In MICROSECONDS
     */
    static inline const Time keyFrameSaveFrequency = 50000;
    Time lastKeyFrameExportDate = 0; //!< Date of the last key frame export

    ofstream* exportFile = nullptr;     //!< binary export file
    ofstream* debugFile = nullptr;      //!< corresponding clear text export file for debugging

    /**
     * Position of the start of the key frames index in the output file
     *  Used to create the keyFrame index table at the end of the simulation export
     */
    streampos keyFramesIndexPos;
    streampos keyFramesIndexPosDebug; //!< @see keyFramesIndexPos but for clear-text debug file

    /**
     * Keeps track of all key frames that have been written to the files.
     *  matching their position in the file to their date in the simulation
     */
    std::map<Time, streampos> keyFramesIndex;

    /**
     * @return a filename string with format replay_<appName>_<confName>_timestamp.vs
     */
    string buildExportFilename() const;

    /**
     * Essentially replaces the default extension in exportFn by ".txt"
     * @param exportFn
     * @return Transform export filename exportFn to debug filename
     */
    string debugFilenameFromExportFilename(const string& exportFn) const;
public:
    /**
     * Constructor for the abstract configuration exporter
     * Creates and writes the binary header for the simulation data file
     */
    ReplayExporter();
    virtual ~ReplayExporter() {}

    /**
     * @return the binary export file instance
     */
    std::ofstream& getExportFile() const {
        if (exportFile == nullptr)
            throw BaseSimulator::VisibleSimException("(error) ReplayExporter::getExportFile(): ReplayExporter has not been initialized");

        return *exportFile;
    }

    /**
     * @return the clear-text export file instance
     */
    std::ofstream& getDebugFile() const {
        if (exportFile == nullptr)
            throw BaseSimulator::VisibleSimException("(error) ReplayExporter::getDebugFile(): ReplayExporter has not been initialized");

        return *debugFile;
    }

    /**
     * [HEADER]
     * Writes the replay file's header, using the following format:
     *
     * [VS_MAGIC][MODULE_TYPE][GRID DIMENSIONS XYZ]
     *
     */
    void writeHeader();


    /**
     * Writes an index with the position of all keyFrames in the replay file,
     *  indexed by their time
     */
    void writeKeyFramesIndex();


    /**
     * Compares the date of the last keyframe export with the current simulation date,
     *  and calls writeKeyFrame if date > (lastKFDate + kfFrequency) or if date = 0
     * @see writeKeyFrame
     * @see saveKeyFrameFrequency
     * @see // lastKeyFrameExportDate
     * @param date current simulation date
     */
    void writeKeyFrameIfNeeded(Time date);

    /**
     * Writes a key frame to the export file and save its location to the index.
     * Key frame format:
     * [Number of modules in frame]
     * > Then for each module (using BuildingBlock::serialize)):
     * [BID][Position XYZ][Orientation][Color RGB]
     *
     *
     * @param date key frame date
     * @see BuildingBlock::serialize
     */
    void writeKeyFrame(Time date);

    /**
     * Terminates simulation replay export by exporting key frame index
     *  and properly closes associated files
     * @note Is called at scheduler end by default, or when simulator is deleted
     */
    void endExport();

    /**
     * Write a color update event to the replay file(s)
     *
     * Format: [date][bid][color rgb]
     *
     * @param date event date
     * @param bid concerned module id
     * @param color new color
     */
    void writeColorUpdate(Time date, bID bid, const Color& color);

    /**
     * Singleton getter
     * @return the singleton instance
     */
    static ReplayExporter* getInstance() {
        if (not singleton)
            singleton = new ReplayExporter();

        return singleton;
    }

    /**
     * @return true if simulation data is being exported for this instance of the simulation
     */
    static bool isReplayEnabled();
};
