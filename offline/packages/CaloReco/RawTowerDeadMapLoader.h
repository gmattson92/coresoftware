// $Id: $

/*!
 * \file RawTowerDeadMapLoader.h
 * \brief
 * \author Jin Huang <jhuang@bnl.gov>
 * \version $Revision:   $
 * \date $Date: $
 */

#ifndef CALORECO_RAWTOWERDEADMAPLOADER_H
#define CALORECO_RAWTOWERDEADMAPLOADER_H

#include <fun4all/SubsysReco.h>

#include <string>

class PHCompositeNode;
class CDBTTree;

/*!
 * \brief RawTowerDeadMapLoader loads dead map at inti run
 */
class RawTowerDeadMapLoader : public SubsysReco
{
 public:
  explicit RawTowerDeadMapLoader(const std::string& detector);

  ~RawTowerDeadMapLoader() override {}

  int InitRun(PHCompositeNode* topNode) override;

  const std::string& detector() const
  {
    return m_detector;
  }

  void detector(const std::string& detector)
  {
    m_detector = detector;
  }

 private:
  std::string m_detector;
  CDBTTree *m_CDBTTree = nullptr;
};

#endif
