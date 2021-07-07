#ifndef PANOSC_ILL_INSTRUMENT_HH
#define PANOSC_ILL_INSTRUMENT_HH
namespace panosc
{

/** \brief implemented instruments */
enum instrument_t {
	D22 = 0,       /** D22 Detector */
	notIMPLEMENTED /** not implemented/not defined */
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// map instrument_t values to JSON as strings
NLOHMANN_JSON_SERIALIZE_ENUM(instrument_t, {
                                               {D22, "D22"},
                                           })
#endif

} // namespace panosc

#endif
