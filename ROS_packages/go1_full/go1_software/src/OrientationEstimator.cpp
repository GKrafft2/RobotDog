#include "../include/OrientationEstimator.h"

/*!
 * Get quaternion, rotation matrix, angular velocity (body and world),
 * rpy, acceleration (world, body) from vector nav IMU
 */
void VectorNavOrientationEstimator::run() {
  //std::cout << "orientation" << std::endl;
  this->_stateEstimatorData.result->orientation[0] =
      lowState.imu.quaternion[0];
  this->_stateEstimatorData.result->orientation[1] =
      lowState.imu.quaternion[1];
  this->_stateEstimatorData.result->orientation[2] =
      lowState.imu.quaternion[2];
  this->_stateEstimatorData.result->orientation[3] =
      lowState.imu.quaternion[3];

  this->_stateEstimatorData.result->rBody = ori::quaternionToRotationMatrix(
      this->_stateEstimatorData.result->orientation);

  this->_stateEstimatorData.result->omegaBody(0) =
      lowState.imu.gyroscope[0];
  this->_stateEstimatorData.result->omegaBody(1) =
      lowState.imu.gyroscope[1];
  this->_stateEstimatorData.result->omegaBody(2) =
      lowState.imu.gyroscope[2];
      this->_stateEstimatorData.result->rpy =
    ori::quatToRPY(this->_stateEstimatorData.result->orientation);

  this->_stateEstimatorData.result->omegaWorld =
      this->_stateEstimatorData.result->rBody.transpose() *
      this->_stateEstimatorData.result->omegaBody;

  this->_stateEstimatorData.result->rpy =
      ori::quatToRPY(this->_stateEstimatorData.result->orientation);
      
  this->_stateEstimatorData.result->aBody(0) =
      lowState.imu.accelerometer[0];
  this->_stateEstimatorData.result->aBody(1) =
      lowState.imu.accelerometer[1];   
  this->_stateEstimatorData.result->aBody(2) =
      lowState.imu.accelerometer[2];

  this->_stateEstimatorData.result->aWorld =
      this->_stateEstimatorData.result->rBody.transpose() *
      this->_stateEstimatorData.result->aBody;
}

/*!
 * Get quaternion, rotation matrix, angular velocity (body and world),
 * rpy, acceleration (world, body) by copying from cheater state data
 */
void CheaterOrientationEstimator::run() {
  this->_stateEstimatorData.result->orientation[0] = lowState.cheat.orientation[0];
  this->_stateEstimatorData.result->orientation[1] = lowState.cheat.orientation[1];
  this->_stateEstimatorData.result->orientation[2] = lowState.cheat.orientation[2];
  this->_stateEstimatorData.result->orientation[3] = lowState.cheat.orientation[3];

  this->_stateEstimatorData.result->rBody = ori::quaternionToRotationMatrix(
      this->_stateEstimatorData.result->orientation);

  this->_stateEstimatorData.result->omegaBody[0] = lowState.cheat.omegaBody[0];
  this->_stateEstimatorData.result->omegaBody[1] = lowState.cheat.omegaBody[1];
  this->_stateEstimatorData.result->omegaBody[2] = lowState.cheat.omegaBody[2];

  this->_stateEstimatorData.result->omegaWorld = 
    this->_stateEstimatorData.result->rBody.transpose() *
    this->_stateEstimatorData.result->omegaBody;
  this->_stateEstimatorData.result->rpy =
    ori::quatToRPY(this->_stateEstimatorData.result->orientation);

  this->_stateEstimatorData.result->aBody[0] = lowState.imu.accelerometer[0];
  this->_stateEstimatorData.result->aBody[1] = lowState.imu.accelerometer[1];
  this->_stateEstimatorData.result->aBody[2] = lowState.imu.accelerometer[2];

  this->_stateEstimatorData.result->aWorld =
      this->_stateEstimatorData.result->rBody.transpose() *
      this->_stateEstimatorData.result->aBody;
}