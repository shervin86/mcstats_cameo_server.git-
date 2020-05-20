#ifndef STAGES_HEADER
#define STAGES_HEADER
#include <map>
#include <string>
/** \ingroup clientAPI
 * global definitions */
namespace panosc_sim_server
{

/** stages implemented as integers to allow for loops
 */
typedef size_t stage_t;
/**
 * @{
 */
static const stage_t sFULL     = 2; ///< FULL detector simulation stage
static const stage_t sSAMPLE   = 1; ///< simulation from source, excluding the sample and following parts
static const stage_t sDETECTOR = 0; ///< simulation including the sample, not the detector
                                    /// @}

///\brief here's the string names of the stages, useful for output directories, etc.
// static const std::vector<std::string> stages = {"sDETECTOR", "sSAMPLE", "sFULL"};
static const std::map<stage_t, std::string> stages = {
    {sDETECTOR, "sDETECTOR"}, {sSAMPLE, "sSAMPLE"}, {sFULL, "sFULL"}};
} // namespace panosc_sim_server
#endif
