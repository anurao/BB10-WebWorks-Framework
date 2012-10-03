/*
 * Copyright 2012 Research In Motion Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

describe("blackberry.sensors", function () {
        
    describe("onsensor", function () {
        var onSensor = jasmine.createSpy(),
            waitForTimeout = 3000;

        it('should invoke callback when sensor is registered', function () {
            runs(function () {
                blackberry.event.addEventListener("onsensor", onSensor);
                blackberry.sensors.startSensor(blackberry.sensors.ACCELEROMETER, { delay : 1000 });
            });

            waitsFor(function () {
                return onSensor.callCount;
            }, "event never fired", waitForTimeout);

            runs(function () {
                expect(onSensor).toHaveBeenCalled();
            });
        });

        it('should not invoke callback when sensor is unregistered', function () {
            runs(function () {
                blackberry.event.removeEventListener("onsensor", onSensor);
            });

            onSensor.reset();
            waits(1000);

            runs(function () {
                expect(onSensor).not.toHaveBeenCalled();
            });
        });
    });
    
    describe("sensors", function () {
        var onSensor = jasmine.createSpy(),
            waitForTimeout = 5000;
        
        beforeEach(function () {
            waits(1000);
        });
        
        runs(function () {
            blackberry.event.addEventListener("onsensor", onSensor);
        });
            
        it('should be able to activate the accelerometer sensor and get valid data', function () {
            runs(function () {
                blackberry.sensors.startSensor(blackberry.sensors.ACCELEROMETER, { delay : 1000 });
            });

            waitsFor(function () {
                return onSensor.callCount;
            }, "event never fired", waitForTimeout);

            runs(function () {
                expect(onSensor).toHaveBeenCalledWith(blackberry.sensors.ACCELEROMETER, jasmine.any(Object));
                blackberry.sensors.stopSensor(blackberry.sensors.ACCELEROMETER);
            });
        });
        
        it('should be able to activate the magnetometer sensor and get valid data', function () {
            runs(function () {
                onSensor.reset();
                blackberry.sensors.startSensor(blackberry.sensors.MAGNETOMETER, { delay : 1000 });
            });

            waitsFor(function () {
                return onSensor.callCount;
            }, "event never fired", waitForTimeout);

            runs(function () {
                expect(onSensor).toHaveBeenCalledWith(blackberry.sensors.MAGNETOMETER, jasmine.any(Object));
                blackberry.sensors.stopSensor(blackberry.sensors.MAGNETOMETER);
            });
        });
            
        it('should be able to activate the gyroscope sensor and get valid data', function () {
            runs(function () {
                onSensor.reset();
                blackberry.sensors.startSensor(blackberry.sensors.GYROSCOPE, { delay : 1000 });
            });

            waitsFor(function () {
                return onSensor.callCount;
            }, "event never fired", waitForTimeout);

            runs(function () {
                expect(onSensor).toHaveBeenCalledWith(blackberry.sensors.GYROSCOPE, jasmine.any(Object));
                blackberry.sensors.stopSensor(blackberry.sensors.GYROSCOPE);
            });
        });

        it('should be able to activate the compass sensor and get valid data', function () {
            runs(function () {
                onSensor.reset();
                blackberry.sensors.startSensor(blackberry.sensors.COMPASS, { delay : 1000 });
            });

            waitsFor(function () {
                return onSensor.callCount;
            }, "event never fired", waitForTimeout);

            runs(function () {
                expect(onSensor).toHaveBeenCalledWith(blackberry.sensors.COMPASS, jasmine.any(Object));
                blackberry.sensors.stopSensor(blackberry.sensors.COMPASS);
            });
        });
        
        it('should be able to activate the proxmity sensor and get valid data', function () {
            runs(function () {
                onSensor.reset();
                blackberry.sensors.startSensor(blackberry.sensors.PROXIMITY, { delay : 1000 });
            });

            waitsFor(function () {
                return onSensor.callCount;
            }, "event never fired", waitForTimeout);

            runs(function () {
                expect(onSensor).toHaveBeenCalledWith(blackberry.sensors.PROXIMITY, jasmine.any(Object));
                blackberry.sensors.stopSensor(blackberry.sensors.PROXIMITY);
            });
        });
        
        it('should be able to activate the light sensor and get valid data', function () {
            runs(function () {
                onSensor.reset();
                blackberry.sensors.startSensor(blackberry.sensors.LIGHT, { delay : 1000 });
            });

            waitsFor(function () {
                return onSensor.callCount;
            }, "event never fired", waitForTimeout);

            runs(function () {
                expect(onSensor).toHaveBeenCalledWith(blackberry.sensors.LIGHT, jasmine.any(Object));
                blackberry.sensors.stopSensor(blackberry.sensors.LIGHT);
            });
        });
        
        it('should be able to activate the gravity sensor and get valid data', function () {
            runs(function () {
                onSensor.reset();
                blackberry.sensors.startSensor(blackberry.sensors.GRAVITY, { delay : 1000 });
            });

            waitsFor(function () {
                return onSensor.callCount;
            }, "event never fired", waitForTimeout);

            runs(function () {
                expect(onSensor).toHaveBeenCalledWith(blackberry.sensors.GRAVITY, jasmine.any(Object));
                blackberry.sensors.stopSensor(blackberry.sensors.GRAVITY);
            });
        });
        
        
        it('should be able to activate the linear acceleration sensor and get valid data', function () {
            runs(function () {
                onSensor.reset();
                blackberry.sensors.startSensor(blackberry.sensors.LINEAR_ACCELERATION, { delay : 1000 });
            });

            waitsFor(function () {
                return onSensor.callCount;
            }, "event never fired", waitForTimeout);

            runs(function () {
                expect(onSensor).toHaveBeenCalledWith(blackberry.sensors.LINEAR_ACCELERATION, jasmine.any(Object));
                blackberry.sensors.stopSensor(blackberry.sensors.LINEAR_ACCELERATION);
            });
        });
        
        it('should be able to activate the rotation vector sensor and get valid data', function () {
            runs(function () {
                onSensor.reset();
                blackberry.sensors.startSensor(blackberry.sensors.ROTATION_VECTOR, { delay : 1000 });
            });

            waitsFor(function () {
                return onSensor.callCount;
            }, "event never fired", waitForTimeout);

            runs(function () {
                expect(onSensor).toHaveBeenCalledWith(blackberry.sensors.ROTATION_VECTOR, jasmine.any(Object));
                blackberry.sensors.stopSensor(blackberry.sensors.ROTATION_VECTOR);
            });
        });
        
        it('should be able to activate the orientation sensor and get valid data', function () {
            runs(function () {
                onSensor.reset();
                blackberry.sensors.startSensor(blackberry.sensors.ORIENTATION, { delay : 1000 });
            });

            waitsFor(function () {
                return onSensor.callCount;
            }, "event never fired", waitForTimeout);

            runs(function () {
                expect(onSensor).toHaveBeenCalledWith(blackberry.sensors.ORIENTATION, jasmine.any(Object));
                blackberry.sensors.stopSensor(blackberry.sensors.ORIENTATION);
            });
        });        

        it('should be able to activate the azimuth/pitch/roll sensor and get valid data', function () {
            runs(function () {
                onSensor.reset();
                blackberry.sensors.startSensor(blackberry.sensors.AZIMUTH_PITCH_ROLL, { delay : 1000 });
            });

            waitsFor(function () {
                return onSensor.callCount;
            }, "event never fired", waitForTimeout);

            runs(function () {
                expect(onSensor).toHaveBeenCalledWith(blackberry.sensors.AZIMUTH_PITCH_ROLL, jasmine.any(Object));
                blackberry.sensors.stopSensor(blackberry.sensors.AZIMUTH_PITCH_ROLL);
            });
        });        

        it('should be able to activate the face detection sensor and get valid data', function () {
            runs(function () {
                onSensor.reset();
                blackberry.sensors.startSensor(blackberry.sensors.FACE_DETECT, { delay : 1000 });
            });

            waitsFor(function () {
                return onSensor.callCount;
            }, "event never fired", waitForTimeout);

            runs(function () {
                expect(onSensor).toHaveBeenCalledWith(blackberry.sensors.FACE_DETECT, jasmine.any(Object));
                blackberry.sensors.stopSensor(blackberry.sensors.FACE_DETECT);
            });
        });        
        
        it('should be able to activate the holster sensor and get valid data', function () {
            runs(function () {
                onSensor.reset();
                blackberry.sensors.startSensor(blackberry.sensors.HOLSTER, { delay : 1000 });
            });

            waitsFor(function () {
                return onSensor.callCount;
            }, "event never fired", waitForTimeout);

            runs(function () {
                expect(onSensor).toHaveBeenCalledWith(blackberry.sensors.HOLSTER, jasmine.any(Object));
                blackberry.sensors.stopSensor(blackberry.sensors.HOLSTER);
            });
        });        
        
    });

});

