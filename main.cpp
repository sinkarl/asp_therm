#include "target_sys.h"

#include "gas_by_file.h"
#include "gasmix_by_file.h"
#include "model_redlich_kwong.h"
#include "model_peng_robinson.h"
#include "models_creator.h"
#include "xml_reader.h"

#include <vector>

#include <assert.h>

#if defined (_OS_NIX)
#  include <unistd.h>
#elif defined(_OS_WIN)
#  include <direct.h>
#  define getcwd _getcwd
#endif  // _OS

/// gas example input
///  gas   |vol,m^3/kg| p,Pa*10^7 |  T,K  |molmass,kg/mol| adiabat,1| cv, J/(kg*K)| acentric factor,1
/// methan  0.00617     4.641       190.66     16.043       ~1.3      ~1750           0.011
/// ethane  0.004926    4.871       305.33     30.07        ~1.22     ~1750           0.089
/// propane 0.004545    4.255       369.9      44.097       ~1.13     ~1750           0.153

// #define RK2_TEST
#define PR_TEST

#ifdef _IDE_VSCODE
const std::string xml_path = "/../asp_therm/data/gases/";
#else
const std::string xml_path = "/../../asp_therm/data/gases/";
#endif  // IDE_VSCODE
const std::string xml_methane = "methane.xml";
const std::string xml_ethane  = "ethane.xml";
const std::string xml_propane = "propane.xml";

const std::string xml_gasmix = "gasmix_inp_example.xml";

int test_models() {
  char cwd[512] = {0};
  if (!getcwd(cwd, (sizeof(cwd)))) {
    std::cerr << "cann't get current dir";
    return 1;
  }
  std::string filename = std::string(cwd) + xml_path + xml_methane;
  /*
  std::unique_ptr<ComponentByFile> met_xml(ComponentByFile::Init(filename));
  if (met_xml == nullptr) {
    std::cerr << "Object of XmlFile wasn't created\n";
    return 1;
  }
  auto cp = met_xml->GetConstParameters();
  auto dp = met_xml->GetDynParameters();
  if (cp == nullptr) {
    std::cerr << "const_parameters by xml wasn't created\n";
    return 2;
  }
  if (dp == nullptr) {
    std::cerr << "dyn_parameters by xml wasn't created\n";
    return 2;
  }
  */
  // ссылочку а не объект
  // PhaseDiagram &pd = PhaseDiagram::GetCalculated();
  // binodalpoints *bp = pd.GetBinodalPoints(cp->V_K, cp->P_K, cp-> T_K,
  //     rg_model_t::REDLICH_KWONG2, cp->acentricfactor);
#if defined(RK2_TEST)
  Redlich_Kwong2 *calc_mod = Redlich_Kwong2::Init(modelName::REDLICH_KWONG2,
      {1.42, 100000, 275}, *cp, *dp, bp);
#elif defined(PR_TEST)
  std::unique_ptr<modelGeneral> calc_mod(ModelsCreator::GetCalculatingModel(
      rg_model_t::PENG_ROBINSON, filename));
  // = Peng_Robinson::Init(set_input(
  //    rg_model_t::PENG_ROBINSON, bp, 100000, 275, *cp, *dp));
#endif  // _TEST
  std::cerr << calc_mod->ParametersString() << std::flush;
  std::cerr << "Set volume(10e6, 314)\n" << std::flush;
  calc_mod->SetVolume(1000000, 314);
  std::cerr << calc_mod->ConstParametersString() << std::flush;
  std::cerr << modelGeneral::sParametersStringHead() << std::flush;
  std::cerr << calc_mod->ParametersString() << std::flush;
//  std::cerr << "\n vol  " << calc_mod->GetVolume(25000000, 365.85) <<
//      "\n pres " << calc_mod->GetPressure(0.007267, 365.85) << std::flush;
  std::cerr << "Push any key to finish";
  std::getchar();
  return 0;
}

int test_models_mix() {
  char cwd[512] = {0};
  if (!getcwd(cwd, (sizeof(cwd)))) {
    std::cerr << "cann't get current dir";
    return 1;
  }
  std::vector<gasmix_file> xml_files = std::vector<gasmix_file> {
    gasmix_file(std::string(cwd) + xml_path + xml_methane, 0.988),
    // add more (summ = 1.00)
    gasmix_file(std::string(cwd) + xml_path + xml_ethane, 0.009),
    gasmix_file(std::string(cwd) + xml_path + xml_propane, 0.003)
  };
  /*
  std::unique_ptr<XmlFile> met_xml(XmlFile::Init(methane_path));
  if (met_xml == nullptr) {
    std::cerr << "Object of XmlFile wasn't created\n";
    return 1;
  }
  */
  /*
  GasMixByFiles *gm = GasMixByFiles::Init(xml_files);
  if (!gm) {
    std::cerr << "GasMix init error" << std::flush;
    return 1;
  }
  std::shared_ptr<parameters_mix> prs_mix = gm->GetParameters();
  if (!prs_mix) {
    std::cerr << "GasMix prs_mix error" << std::flush;
    return 1;
  }
  PhaseDiagram &pd = PhaseDiagram::GetCalculated();
  binodalpoints bp = pd.GetBinodalPoints(*prs_mix, rg_model_t::PENG_ROBINSON);
  */
#if defined(RK2_TEST)
  Redlich_Kwong2 *calc_mod = Redlich_Kwong2::Init(modelName::REDLICH_KWONG2,
      {1.42, 100000, 275}, *prs_mix, bp);
#elif defined(PR_TEST)
  std::unique_ptr<modelGeneral> calc_mod(ModelsCreator::GetCalculatingModel(
      rg_model_t::PENG_ROBINSON, xml_files));
  // Peng_Robinson *calc_mod = Peng_Robinson::Init(set_input(rg_model_t::PENG_ROBINSON, bp,
  //     100000, 275, *prs_mix));
#endif  // _TEST
  std::cerr << calc_mod->ConstParametersString() << std::flush;
  std::cerr << modelGeneral::sParametersStringHead() << std::flush;
  std::cerr << calc_mod->ParametersString() << std::flush;
  calc_mod->SetVolume(1000000, 314);
  std::cerr << "Set volume(10e6, 314)\n" << std::flush;
  std::cerr << calc_mod->ParametersString() << std::flush;
  return 0;
}

int main() {
  if (test_models()) {
    std::cerr << "test_models()";
    return 1;
  }
  if (test_models_mix()) {
    std::cerr << "test_models_mix()";
    return 2;
  }
  return 0;
}
