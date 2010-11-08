/*
 * Copyright 2010,
 *
 * Olivier Stasse
 *
 * JRL, CNRS/AIST
 *
 * This file is part of walkGenJrl.
 * walkGenJrl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * walkGenJrl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Lesser Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License
 * along with walkGenJrl.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Research carried out within the scope of the
 *  Joint Japanese-French Robotics Laboratory (JRL)
 */
/* \file This file tests M. Morisawa's walking algorithm for
 * real-time CoM and ZMP trajectory generation
 */

#include "CommonTools.h"
#include "TestObject.h"

using namespace::PatternGeneratorJRL;
using namespace::PatternGeneratorJRL::TestSuite;
using namespace std;

enum Profiles_t {
  PROFIL_ANALYTICAL_ONLINE_WALKING,                 // 1
  PROFIL_ANALYTICAL_SHORT_STRAIGHT_WALKING          //  2
};

#define NBOFPREDEFONLINEFOOTSTEPS 11


double OnLineFootSteps[NBOFPREDEFONLINEFOOTSTEPS][3]={
  { 0.05, 0.0, 0.0},
  { 0.05, 0.0, 0.0},
  { 0.05, 0.0, 0.0},
  { 0.05, 0.0, 0.0},
  { 0.05, 0.0, 0.0},
  { 0.05, 0.0, 0.0},
  { 0.05, 0.0, 0.0},
  { 0.05, 0.0, 0.0},
  { 0.05, 0.0, 0.0},
  { 0.05, 0.0, 0.0},
  { 0.05, 0.0, 0.0}
};

class TestMorisawa2007: public TestObject
{

private:
  bool m_TestChangeFoot;
  unsigned long int m_NbStepsModified;
  // New time between two steps.
  double m_deltatime;
public:
  TestMorisawa2007(int argc, char*argv[], string &aString, int TestProfile):
    TestObject(argc, argv, aString)
  {
    m_TestProfile = TestProfile;
    m_TestChangeFoot = true;
    m_NbStepsModified = 0;
    m_deltatime = 0;
  };

protected:

  void StartAnalyticalOnLineWalking(PatternGeneratorInterface &aPGI)
  {
    CommonInitialization(aPGI);

    {
      istringstream strm2(":SetAlgoForZmpTrajectory Morisawa");
      aPGI.ParseCmd(strm2);
    }

    {
      istringstream strm2(":onlinechangestepframe relative");
      aPGI.ParseCmd(strm2);
    }

    {
      istringstream strm2(":SetAutoFirstStep false");
      aPGI.ParseCmd(strm2);
    }

    {
      istringstream strm2(":StartOnLineStepSequencing 0.0 -0.095 0.0 \
                     0.0 0.19 0.0				     \
                     0.0 -0.19 0.0				     \
                     0.0 0.19 0.0");
      aPGI.ParseCmd(strm2);
    }
  }

  void StopOnLineWalking(PatternGeneratorInterface &aPGI)
  {
    istringstream strm2(":StopOnLineStepSequencing");
    aPGI.ParseCmd(strm2);
  }

  void AnalyticalShortStraightWalking(PatternGeneratorInterface &aPGI)
  {
    CommonInitialization(aPGI);
    {
      istringstream strm2(":SetAlgoForZmpTrajectory Morisawa");
      aPGI.ParseCmd(strm2);
    }

    {
      istringstream strm2(":stepseq 0.0 -0.105 0.0 \
                     0.2  0.19 0.0		   \
                     0.2 -0.19 0.0		   \
                     0.2  0.19 0.0                 \
                     0.2 -0.19 0.0                 \
                     0.2  0.19 0.0		   \
                     0.0 -0.19 0.0");
      aPGI.ParseCmd(strm2);
    }

  }

  void chooseTestProfile()
  {

    switch(m_TestProfile)
      {
      case PROFIL_ANALYTICAL_SHORT_STRAIGHT_WALKING:
	AnalyticalShortStraightWalking(*m_PGI);
	break;

      case PROFIL_ANALYTICAL_ONLINE_WALKING:
	StartAnalyticalOnLineWalking(*m_PGI);
	break;
      default:
	throw("No correct test profile");
	break;
      }
  }

  void generateEvent()
  {
    if (m_TestProfile==PROFIL_ANALYTICAL_SHORT_STRAIGHT_WALKING)
      return;

    unsigned int StoppingTime = 70*200;


    double r = 100.0*(double)m_OneStep.NbOfIt/(double)StoppingTime;


    if (m_OneStep.NbOfIt>StoppingTime) /* Stop after 30 seconds the on-line stepping */
      {
	StopOnLineWalking(*m_PGI);
      }
    else{
      if (m_OneStep.NbOfIt%200==0)
	{
	  cout << "Progress " << (unsigned int)r << "\%"<< "\r";
	  cout.flush();
	}

      double triggertime = 9.64*200 + m_deltatime*200;
      if ((m_OneStep.NbOfIt>triggertime) &&
	  m_TestChangeFoot)
	{
	  PatternGeneratorJRL::FootAbsolutePosition aFAP;
	  if (m_NbStepsModified<NBOFPREDEFONLINEFOOTSTEPS)
	    {
	      aFAP.x = OnLineFootSteps[m_NbStepsModified][0];
	      aFAP.y = OnLineFootSteps[m_NbStepsModified][1];
	      aFAP.theta = OnLineFootSteps[m_NbStepsModified][2];
	    }
	  else
	    {
	      aFAP.x=0.1;
	      aFAP.y=0.0;
	      aFAP.theta=5.0;
	    }
	  double newtime;
	  m_PGI->ChangeOnLineStep(0.805,aFAP,newtime);
	  m_deltatime += newtime+0.025;
	  m_TestChangeFoot=true;
	  m_NbStepsModified++;
	  if (m_NbStepsModified==360)
	    m_TestChangeFoot=false;
	}
    }
  };

};

int PerformTests(int argc, char *argv[])
{

  std::string TestNames[2] = { "TestMorisawa2007OnLine",
			       "TestMorisawa2007ShortWalk"};
  int TestProfiles[2] = { PROFIL_ANALYTICAL_ONLINE_WALKING,
			  PROFIL_ANALYTICAL_SHORT_STRAIGHT_WALKING};

  for (unsigned int i=0;i<2;i++)
    {
        TestMorisawa2007 aTM2007(argc,argv,
				 TestNames[i],
				 TestProfiles[i]);
	try
	  {
	    if (!aTM2007.doTest(std::cout))
	      {
		cout << "Failed test " << i << endl;
		return -1;
	      }
	    else
	      cout << "Passed test " << i << endl;
	  }
	catch (const char * astr)
	  { cerr << "Failed on following error " << astr << std::endl;
	    return -1; }
    }
  return 0;
}

int main(int argc, char *argv[])
{
  try
    {
      return PerformTests(argc,argv);
    }
  catch (const std::string& msg)
    {
      std::cerr << msg << std::endl;
    }
  return 1;
}

