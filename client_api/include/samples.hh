#ifndef PANOSC_SAMPLE_HH
#define PANOSC_SAMPLE_HH

namespace panosc
{

/** \brief list of implemented materials */
enum sample_material_t {
	H2O, /** water */
	D2O  /** deuterium */
};
// cannot use enum class with nlohmann json

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// map sample_material_t values to JSON as strings
NLOHMANN_JSON_SERIALIZE_ENUM(sample_material_t, {
                                                    {H2O, "H2O"},
                                                    {D2O, "D2O"},
                                                })
#endif

#ifdef shervin
class sample
{
	public:
	sample(void){};

	/**@{
	 * Sample's shape, units are in \br m
	 */
	void sphere(double r, double thickness = 0);
	void hollow_sphere(double r, double thickness);
	void cylinder(double r, double h, double thickness = 0);
	void hollow_cylinder(double r, double h, double thickness);
	void box(double x, double y, double z, double thickness = 0);
	void hollow_box(double x, double y, double z, double thickness);
	void shape_file(std::string filename); ///< reads from OFF files
	///@}

	/**@{ Sample's material */
	void crystal();
	void liquid(sample_t material);
	void gaz();
	void powder();
	///@}
};
#endif
} // namespace panosc

#endif
