#ifndef STAGES_HEADER
#define STAGES_HEADER
#include <map>
#include <string>
/** \namespace panosc
 * \brief namespace for simualation platform
 */
/* \ingroup clientAPI */
namespace panosc
{

/** stages implemented as integers to allow for loops
 */
typedef size_t stage_t;
/** \ingroup stages
 * @{
 */
static const stage_t sFULL     = 0;  ///< FULL detector simulation stage // this should always be the last!
static const stage_t sSAMPLE   = 1;  ///< simulation from source, excluding the sample and following parts
static const stage_t sDETECTOR = 2;  ///< simulation including the sample, not the detector
static const stage_t sSOURCE   = 3;  ///< stage not implemented, used to start the loop
static const stage_t sNONE     = -1; ///< stage not implemented, to define not implemented parameters
/// @}

///\brief here's the string names of the stages, useful for output directories, etc.
static const std::map<stage_t, std::string> stages = {
    {sNONE, "sNONE"}, {sSOURCE, "sSOURCE"}, {sDETECTOR, "sDETECTOR"}, {sSAMPLE, "sSAMPLE"}, {sFULL, "sFULL"}};
} // namespace panosc

#endif
