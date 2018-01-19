/*! @file target.h
 * @brief Defines a target configuration for reconfiguration algorithms, 
 * several ways of defining the configuration are provided to the user.
 *
 * e.g. Definition of a list of targets in the XML file:
 *
 *      <targetList>
 *        <target format="grid">
 *          <cell position="0,0,0"/>
 *          <cell position="1,2,3" color="128,128,128"/>
 *          <cell color="128,128,128" position="3,2,1"/>
 *        </target>
 *        <target format="grid">
 *          <cell position="0,0,0"/>
 *          <cell position="1,1,1"/>
 *          <cell position="1,3,2"/>
 *          <cell position="2,2,2"/>
 *          <cell position="3,3,3"/>            
 *        </target>
 *        <target format="csg">
 *          <csg content="union() { cube([10,10,10]); cylinder(10,5,5); }"/>
 *        </target>
 *        </targetList>
 *
 * @author Pierre Thalamy
 * @date 21/07/2016
 */

#ifndef TARGET_H__
#define TARGET_H__

#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <Eigen/Dense>
#include <cfloat>

#define TIXML_USE_STL	1
#include "TinyXML/tinyxml.h"

#include "color.h"
#include "cell3DPosition.h"
#include "csg.h"
#include "vector3D.h"

using namespace std;

namespace BaseSimulator {

//<! @brief Abstract Target. Provides the user with functions for checking a target position and color.
class Target {    
protected:
    
    /**
     * @brief prints target to an ouput string
     * @param where ostream on which to print the object
     */
    virtual void print(ostream& where) const {};

public:
    //<! @brief Exception thrown if an error as occured during parsing
    struct TargetParsingException : std::exception {
        const char* what() const noexcept {
            return "Invalid target description in configuration file\n";
        }
    };
    //<! @brief Exception thrown if the user is attempting to check a position that is not part of the target
    struct InvalidPositionException : std::exception {
        const char* what() const noexcept {
            return "Position does not belong to the target\n";
        }
    };
    //<! @brief Exception thrown if the user provides incorrect dimensions for the target
    struct InvalidDimensionsException : std::exception {
        const char* what() const noexcept {
            return "Target dimensions are invalid\n";
        }
    };

    static TiXmlNode *targetListNode; //!< pointer to the target list node from the XML configuration file
    static TiXmlNode *targetNode; //!< pointer to the current target node from the XML configuration file

    /**
     * @brief Parse next target from the configuration file's Target List, and return a pointer to the instantiated object
     * @return pointer to the parsed Target object, or NULL if there are no (more) targets in the configuration file
     */
    static Target *loadNextTarget();    
    /**
     * @brief Target constructor. Where parsing occurs thanks to the targetNode parameter
     * @param targetNode XML Node containing target description from configuration file
     */
    Target(TiXmlNode *targetNode) {};
    virtual ~Target() {};
    
    /**
     * @brief Indicates if a position belongs to the target
     * @param pos position to consider
     * @return true if pos belongs to the target, false otherwise
     */
    virtual bool isInTarget(const Cell3DPosition &pos) = 0;
    /**
     * @brief Returns the target color at position pos
     * @param pos position to condiser
     * @return target color at cell p
     */
    virtual const Color getTargetColor(const Cell3DPosition &pos) = 0;

    /**
     * @brief Returns the target bounding box
     * @param bb boundingbox to be written
     */
    virtual void boundingBox(BoundingBox &bb) = 0;
    
    friend ostream& operator<<(ostream& out,const Target *t);
};  // class Target

//<! @brief A target modeled as a container of unique positions and colors.c
class TargetGrid : public Target {
    // Only store target cells instead of the entire grid to save memory
    map<const Cell3DPosition, const Color> tCells; //!< the target cells as Cell/Color key-value pairs

protected:    
    /**
     * @brief Add a cell to the target cells container
     * @param pos position of the target cell
     * @param c color of the cell. If none provided, defaults to (0,0,0,0)
     */
    void addTargetCell(const Cell3DPosition &pos, const Color c = Color());

    //!< @copydoc Target::print
    virtual void print(ostream& where) const;    
public:
    /**
     * @copydoc Target::Target 
     * XML Description Format:
     * <target format="grid">
     *   <cell position="x,y,z" color="r,g,b"/>
     *   ...
     * </target>
     */
    TargetGrid(TiXmlNode *targetNode);
    virtual ~TargetGrid() {};

    //!< @copydoc Target::getTargetColor
    //!< a cell is in the target grid if and only if it is present in the target cells container
    virtual bool isInTarget(const Cell3DPosition &pos);
    //!< @copydoc Target::getTargetColor
    //!< @throws InvalidPositionException is cell at position pos is not part of the target
    virtual const Color getTargetColor(const Cell3DPosition &pos);

    //!< @copydoc Target::BoundingBox
    virtual void boundingBox(BoundingBox &bb);

};  // class TargetGrid

//<! @brief A target modeled as an ensemble of shapes
class TargetCSG : public Target {
    CSGNode *csgRoot;
    BoundingBox bb;

protected:
    //!< @copydoc Target::print
    virtual void print(ostream& where) const {};
public:
    TargetCSG(TiXmlNode *targetNode);
    virtual ~TargetCSG() {};

    //!< @copydoc Target::isInTarget
    virtual bool isInTarget(const Cell3DPosition &pos);
    //!< @copydoc Target::getTargetColor
    virtual const Color getTargetColor(const Cell3DPosition &pos);
    //!< @copydoc Target::boundingBox
    virtual void boundingBox(BoundingBox &bb);
    /**
     * @brief Grid to world position within bounding box
     * @param pos position of the target cell
     */
    Vector3D gridToWorldPosition(const Cell3DPosition &pos);
    /**
     * @brief The object is in the border of the target
     * @param pos position of the target cell
     * @param radius radius of the border
     */
    bool isInTargetBorder(const Cell3DPosition &pos, double radius);

};  // class TargetCSG

//<! @brief A target modeling a surface by a point cloud
class TargetSurface : public Target {
    // Stores the points of the point cloud
    vector<Vector3D> pcl; //!< the point cloud
    vector<float> coeffs; //!< the coefficients of the interpolating polynom
    string method;
    int S_NUMPOINTS;
    int S_ORDER;
    int S_NUMKNOTS;
    int T_NUMPOINTS;
    int T_ORDER;
    int T_NUMKNOTS;
    vector<float> sknots;
    vector<float> tknots;
    vector<vector<vector<float>>> ctlpoints;

protected:    
    /**
     * @brief Add a cell to the target cells container
     * @param pos position of the target cell
     * @param c color of the cell. If none provided, defaults to (0,0,0,0)
     */
    void addTargetCell(const Cell3DPosition &pos, const Color c = Color());

    float calculateNurbs(float u, float v, int coord);

    float dist(float x1, float y1, float x2, float y2);

    //!< @copydoc Target::print
    virtual void print(ostream& where) const;    
public:
    /**
     * @copydoc Target::Target 
     * XML Description Format:
     * <target format="surface">
     *   <method meth="type">
     *     <cell position="x,y,z" color="r,g,b"/>
     *     ...
     *   </method>
     * </target>
     */
    TargetSurface(TiXmlNode *targetNode);
    virtual ~TargetSurface() {};

    //!< @copydoc Target::getTargetColor
    //!< a cell is in the target grid if it is at the same level or under as the surface described for a couple (x,y)
    virtual bool isInTarget(const Cell3DPosition &pos);
    //!< @copydoc Target::getTargetColor
    //!< @throws InvalidPositionException is cell at position pos is not part of the target
    virtual const Color getTargetColor(const Cell3DPosition &pos);

    //!< @copydoc Target::BoundingBox
    virtual void boundingBox(BoundingBox &bb);

};  // class TargetSurface


} // namespace BaseSimulator

#endif  // TARGET_H__
