#ifndef PANOSC_SAMPLE_HH
#define PANOSC_SAMPLE_HH

namespace panosc
{

/** \brief list of implemented materials */
enum sample_material_t {
	H2O, /** water */
	D2O  /** deuterium */
};
// cannot use enum class with nlohmann json, but simple enum

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// map sample_material_t values to JSON as strings
NLOHMANN_JSON_SERIALIZE_ENUM(sample_material_t, {
                                                    {H2O, "H2O"},
                                                    {D2O, "D2O"},
                                                })
} // namespace panosc

#endif
