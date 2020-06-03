#ifndef STAGES_HEADER
#define STAGES_HEADER
#include <map>
#include <string>
/** \namespace panosc 
 * \brief namespace for simualation platform 
 * \ingroup clientAPI */
namespace panosc
{

/** stages implemented as integers to allow for loops
 */
typedef size_t stage_t;
/** \ingroup clientAPI
 * @{
 */
static const stage_t sFULL     = 3; ///< FULL detector simulation stage
static const stage_t sSAMPLE   = 2; ///< simulation from source, excluding the sample and following parts
static const stage_t sDETECTOR = 1; ///< simulation including the sample, not the detector
static const stage_t sSOURCE   = 0; ///< stage not implemented, used to start the loop

/// @}

///\brief here's the string names of the stages, useful for output directories, etc.
static const std::map<stage_t, std::string> stages = {
    {sSOURCE, "sSOURCE"}, {sDETECTOR, "sDETECTOR"}, {sSAMPLE, "sSAMPLE"}, {sFULL, "sFULL"}};
} // namespace panosc

#endif
