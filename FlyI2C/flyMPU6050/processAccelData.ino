void processAccelData(){
  gForceX = (accelX*9.8)/ 2048.0;
  gForceY = (accelY*9.8)/ 2048.0; 
  gForceZ = (accelZ*9.8)/ 2048.0;
  
  
  /*pitch = 180 * atan(gForceX/sqrt(gForceY*gForceY + gForceZ*gForceZ))/PI;
  roll = 180 * atan(gForceY/sqrt(gForceX*gForceX + gForceZ*gForceZ))/PI;
  yaw = 180 * atan(gForceZ/sqrt(gForceY*gForceY + gForceZ*gForceZ))/PI;
  */
}
