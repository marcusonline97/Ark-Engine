{
  "activeCanvas": "MainCanvas",
  "camera": "MainPlayer",
  "name": "MyScene",
  "objects": [
    {
      "children": [
        {
          "children": [
            {
              "components": [
                {
                  "type": "AnimationComponent"
                }
              ],
              "name": "Gun",
              "path": "models/CarbineGun/scene.gltf",
              "position": {
                "x": 0.75,
                "y": -0.5,
                "z": -0.75
              },
              "rotation": {
                "w": 1.0,
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
              },
              "scale": {
                "x": -1.0,
                "y": 1.0,
                "z": 1.0
              },
              "type": "gltf"
            }
          ],
          "components": [
            {
              "type": "CameraComponent"
            },
            {
              "type": "PlayerControllerComponent"
            },
            {
              "type": "AudioListenerComponent"
            },
            {
              "audio": [
                {
                  "name": "shoot",
                  "path": "audio/WPN_MP7_Fire_04.wav",
                  "volume": 0.2
                },
                {
                  "name": "step",
                  "path": "audio/player_step_1.wav",
                  "volume": 0.6
                },
                {
                  "name": "jump",
                  "path": "audio/player_step_4.wav",
                  "volume": 0.8
                }
              ],
              "type": "AudioComponent",
              "volume": 1.0
            }
          ],
          "name": "MainPlayer",
          "position": {
            "x": 0.0,
            "y": 2.0,
            "z": -7.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          },
          "type": "Player"
        },
        {
          "children": [
            {
              "components": [
                {
                  "type": "AnimationComponent"
                }
              ],
              "name": "PickupGunModel",
              "path": "models/CarbineGun/scene.gltf",
              "position": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
              },
              "rotation": {
                "w": 1.0,
                "x": 0.0,
                "y": 0.5,
                "z": 0.0
              },
              "scale": {
                "x": 1.0,
                "y": 1.0,
                "z": 1.0
              },
              "type": "gltf"
            }
          ],
          "components": [
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "sphere",
                "x": 0.75
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "GunPickup",
          "position": {
            "x": 0.0,
            "y": 1.2000000476837158,
            "z": -3.0
          },
          "rotation": {
            "w": -0.954773485660553,
            "x": 0.0,
            "y": -0.29733434319496155,
            "z": 0.0
          },
          "rotationSpeed": 90.0,
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          },
          "type": "GunPickup"
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 30,
                "y": 1,
                "z": 30
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 30,
                "y": 1,
                "z": 30
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "Ground",
          "position": {
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1,
                "y": 5,
                "z": 30
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 1,
                "y": 5,
                "z": 30
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "LeftWall",
          "position": {
            "x": -15.5,
            "y": 3.0,
            "z": 0.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 2.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1,
                "y": 5,
                "z": 20
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 1,
                "y": 5,
                "z": 20
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "RightWall",
          "position": {
            "x": 15.5,
            "y": 3.0,
            "z": 5.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 2.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1,
                "y": 5,
                "z": 6
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 1,
                "y": 5,
                "z": 6
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "RightWallFrontSegment",
          "position": {
            "x": 15.5,
            "y": 3.0,
            "z": -12.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 3.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 18,
                "y": 1,
                "z": 4
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 18,
                "y": 1,
                "z": 4
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "CorridorFloor",
          "position": {
            "x": 24.0,
            "y": 0.0,
            "z": -7.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 18,
                "y": 5,
                "z": 1
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 18,
                "y": 5,
                "z": 1
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "CorridorLeftWall",
          "position": {
            "x": 24.0,
            "y": 3.0,
            "z": -9.5
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 3.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 18,
                "y": 5,
                "z": 1
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 18,
                "y": 5,
                "z": 1
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "CorridorRightWall",
          "position": {
            "x": 24.0,
            "y": 3.0,
            "z": -4.5
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 3.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1,
                "y": 5,
                "z": 4
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 1,
                "y": 5,
                "z": 4
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "CorridorEndWall",
          "position": {
            "x": 33.5,
            "y": 3.0,
            "z": -7.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1.0,
                      "value1": 0.35,
                      "value2": 0.05
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ShootableCorridorCube1",
          "position": {
            "x": 17.399999618530273,
            "y": 1.25,
            "z": -7.099999904632568
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.2000000476837158,
            "y": 2.0,
            "z": 2.799999952316284
          },
          "type": "ShootableCube"
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1.0,
                      "value1": 0.65,
                      "value2": 0.05
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ShootableCorridorCube2",
          "position": {
            "x": 15.800000190734863,
            "y": 2.0,
            "z": -7.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 2.0,
            "y": 4.0,
            "z": 2.700000047683716
          },
          "type": "ShootableCube"
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1.0,
                      "value1": 0.35,
                      "value2": 0.05
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ShootableCorridorCube3",
          "position": {
            "x": 19.5,
            "y": 1.25,
            "z": -7.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 2.0,
            "y": 2.0,
            "z": 2.700000047683716
          },
          "type": "ShootableCube"
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.8,
                      "value1": 0.2,
                      "value2": 1.0
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ShootableCorridorCube4",
          "position": {
            "x": 22.299999237060547,
            "y": 1.25,
            "z": -6.900000095367432
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 2.0,
            "y": 3.0,
            "z": 2.799999952316284
          },
          "type": "ShootableCube"
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.55,
                      "value1": 0.2,
                      "value2": 1.0
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ShootableCorridorCube5",
          "position": {
            "x": 25.200000762939453,
            "y": 1.649999976158142,
            "z": -7.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 2.0,
            "y": 2.0,
            "z": 3.0
          },
          "type": "ShootableCube"
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.8,
                      "value1": 0.2,
                      "value2": 1.0
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ShootableCorridorCube6",
          "position": {
            "x": 30.299999237060547,
            "y": 2.8499999046325684,
            "z": -7.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 4.0,
            "y": 3.0,
            "z": 3.0
          },
          "type": "ShootableCube"
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 30,
                "y": 5,
                "z": 1
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 30,
                "y": 5,
                "z": 1
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "FrontWall",
          "position": {
            "x": 0.0,
            "y": 3.0,
            "z": -15.5
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 2.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 30,
                "y": 5,
                "z": 1
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 30,
                "y": 5,
                "z": 1
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "BackWall",
          "position": {
            "x": 0.0,
            "y": 3.0,
            "z": 15.5
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 2.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 2,
                "y": 1.1,
                "z": 2
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 2,
                "y": 1.1,
                "z": 2
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "PlatformA",
          "position": {
            "x": -14.0,
            "y": 1.0499999523162842,
            "z": -14.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "LadderA_1",
          "position": {
            "x": -12.850000381469727,
            "y": 1.350000023841858,
            "z": -14.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "LadderA_2",
          "position": {
            "x": -12.550000190734863,
            "y": 1.25,
            "z": -14.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "LadderA_3",
          "position": {
            "x": -12.25,
            "y": 1.149999976158142,
            "z": -14.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "LadderA_4",
          "position": {
            "x": -11.949999809265137,
            "y": 1.0499999523162842,
            "z": -14.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "LadderA_5",
          "position": {
            "x": -11.649999618530273,
            "y": 0.949999988079071,
            "z": -14.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "LadderA_6",
          "position": {
            "x": -11.350000381469727,
            "y": 0.8500000238418579,
            "z": -14.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "LadderA_7",
          "position": {
            "x": -11.050000190734863,
            "y": 0.75,
            "z": -14.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "LadderA_8",
          "position": {
            "x": -10.75,
            "y": 0.6499999761581421,
            "z": -14.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "LadderA_9",
          "position": {
            "x": -10.449999809265137,
            "y": 0.550000011920929,
            "z": -14.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 2,
                "y": 1.1,
                "z": 2
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 2,
                "y": 1.1,
                "z": 2
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "PlatformB",
          "position": {
            "x": -14.0,
            "y": 1.0499999523162842,
            "z": -10.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 2,
                "y": 1.1,
                "z": 8
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 2,
                "y": 1.1,
                "z": 8
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "PlatformС",
          "position": {
            "x": -14.0,
            "y": 1.0499999523162842,
            "z": -3.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 8,
                "y": 1.1,
                "z": 2
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 8,
                "y": 1.1,
                "z": 2
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "PlatformD",
          "position": {
            "x": -7.0,
            "y": 1.0499999523162842,
            "z": -3.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1,
                "y": 2,
                "z": 8
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 1,
                "y": 2,
                "z": 8
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "InternalWall1",
          "position": {
            "x": -1.0,
            "y": 1.5,
            "z": -11.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1,
                "y": 2,
                "z": 8
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 1,
                "y": 2,
                "z": 8
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "InternalWall2",
          "position": {
            "x": 1.5,
            "y": 1.5,
            "z": -1.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1.0,
                      "value1": 0.0,
                      "value2": 0.0
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 2,
                "y": 0.2,
                "z": 2
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 2,
                "y": 0.2,
                "z": 2
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "JumpPlatform",
          "position": {
            "x": -7.0,
            "y": 1.0,
            "z": 1.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          },
          "type": "JumpPlatform"
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.0,
                      "value1": 1.0,
                      "value2": 0.0
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide1",
          "position": {
            "x": 9.999692916870117,
            "y": 1.2499878406524658,
            "z": 0.0006412369548343122
          },
          "rotation": {
            "w": 1.0,
            "x": 9.417138244316448e-06,
            "y": -2.2702001842844766e-06,
            "z": 1.0961064617731608e-05
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.7,
                      "value1": 0.5,
                      "value2": 0.2
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide2",
          "position": {
            "x": 9.999778747558594,
            "y": 1.249968409538269,
            "z": 1.5106151103973389
          },
          "rotation": {
            "w": 1.0,
            "x": 1.554660775582306e-05,
            "y": -1.4273236956796609e-06,
            "z": 1.4070174984226469e-05
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.2,
                      "value1": 0.5,
                      "value2": 0.1
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide3",
          "position": {
            "x": 9.999698638916016,
            "y": 1.2499476671218872,
            "z": 3.0203840732574463
          },
          "rotation": {
            "w": 1.0,
            "x": 1.0706985449360218e-05,
            "y": -5.323746881913394e-05,
            "z": 2.8042224585078657e-05
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.2,
                      "value1": 0.1,
                      "value2": 0.9
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide4",
          "position": {
            "x": 9.999603271484375,
            "y": 1.2499923706054688,
            "z": 4.53000020980835
          },
          "rotation": {
            "w": 1.0,
            "x": -5.353127789931023e-07,
            "y": -1.6545458493055776e-05,
            "z": 9.286146450904198e-06
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1,
                      "value1": 0,
                      "value2": 1
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide5",
          "position": {
            "x": 9.999468803405762,
            "y": 1.2499760389328003,
            "z": 6.0397629737854
          },
          "rotation": {
            "w": 1.0,
            "x": -2.725128751990269e-06,
            "y": 5.909665787839913e-07,
            "z": 2.4064896933850832e-05
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.1,
                      "value1": 0.8,
                      "value2": 0.7
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide6",
          "position": {
            "x": 9.999927520751953,
            "y": 1.2499911785125732,
            "z": 7.55015754699707
          },
          "rotation": {
            "w": 1.0,
            "x": -6.158660653454717e-06,
            "y": 2.58833028965455e-06,
            "z": 1.133738692260522e-06
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.1,
                      "value1": 0.8,
                      "value2": 0.7
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide7",
          "position": {
            "x": 9.999495506286621,
            "y": 2.7499802112579346,
            "z": 0.7522611021995544
          },
          "rotation": {
            "w": 1.0,
            "x": 3.0743358365725726e-05,
            "y": -0.00016486547247041017,
            "z": 1.2271549167053308e-05
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.9,
                      "value1": 0.2,
                      "value2": 0.3
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide8",
          "position": {
            "x": 9.999260902404785,
            "y": 2.7499516010284424,
            "z": 2.2525055408477783
          },
          "rotation": {
            "w": 1.0,
            "x": 1.6132285963976756e-05,
            "y": -0.00012119760503992438,
            "z": 1.4870364793750923e-05
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.2,
                      "value1": 0.2,
                      "value2": 0.3
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide9",
          "position": {
            "x": 9.999436378479004,
            "y": 2.749933958053589,
            "z": 3.752619743347168
          },
          "rotation": {
            "w": 1.0,
            "x": 2.0823565137106925e-05,
            "y": -4.577730942401104e-05,
            "z": 2.3185239115264267e-05
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.5,
                      "value1": 0.1,
                      "value2": 0.9
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide10",
          "position": {
            "x": 9.999446868896484,
            "y": 2.749908447265625,
            "z": 5.2526326179504395
          },
          "rotation": {
            "w": 1.0,
            "x": 1.1568346963031217e-05,
            "y": -4.3195177568122745e-05,
            "z": 3.496049976092763e-05
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1,
                      "value1": 1,
                      "value2": 0
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide11",
          "position": {
            "x": 9.999669075012207,
            "y": 2.749943971633911,
            "z": 6.753572940826416
          },
          "rotation": {
            "w": 1.0,
            "x": -3.5643526643980294e-05,
            "y": 3.640064096543938e-05,
            "z": 2.4257717086584307e-05
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1,
                      "value1": 1,
                      "value2": 0
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide12",
          "position": {
            "x": 9.998880386352539,
            "y": 4.2499284744262695,
            "z": 1.5031439065933228
          },
          "rotation": {
            "w": 1.0,
            "x": 3.076306529692374e-05,
            "y": 5.470620817504823e-05,
            "z": 2.4623412173241377e-05
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1,
                      "value1": 0,
                      "value2": 0
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide13",
          "position": {
            "x": 9.99947452545166,
            "y": 4.249918460845947,
            "z": 3.011457681655884
          },
          "rotation": {
            "w": 1.0,
            "x": 1.716217775538098e-05,
            "y": -5.076980960438959e-05,
            "z": 2.0593628505594097e-05
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0,
                      "value1": 0,
                      "value2": 1
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide14",
          "position": {
            "x": 9.9995698928833,
            "y": 4.249905586242676,
            "z": 4.520536422729492
          },
          "rotation": {
            "w": 1.0,
            "x": 1.9453975255601108e-05,
            "y": -3.6820074456045404e-06,
            "z": 2.5909181204042397e-05
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0,
                      "value1": 1,
                      "value2": 1
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide15",
          "position": {
            "x": 9.999250411987305,
            "y": 4.249900817871094,
            "z": 6.028464317321777
          },
          "rotation": {
            "w": 1.0,
            "x": -3.3991811505984515e-05,
            "y": -3.7931036786176264e-05,
            "z": 2.579595457063988e-05
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1,
                      "value1": 0,
                      "value2": 0
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 2,
                "y": 0.2,
                "z": 2
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 2,
                "y": 0.2,
                "z": 2
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "EndPlatform",
          "position": {
            "x": 30.0,
            "y": 1.0,
            "z": -7.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 2.0,
            "y": 1.100000023841858,
            "z": 2.0
          },
          "type": "JumpPlatform"
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.0,
                      "value1": 1.0,
                      "value2": 0.0
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "r": 1.5,
                "type": "sphere"
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "r": 1.5,
                "type": "sphere"
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide16",
          "position": {
            "x": 10.0,
            "y": 1.499999761581421,
            "z": -8.0
          },
          "rotation": {
            "w": 1.0,
            "x": -6.9293791966629215e-06,
            "y": 2.325720172002832e-11,
            "z": -1.3694383596885018e-05
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.0,
                      "value1": 0.0,
                      "value2": 1.0
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "r": 1.5,
                "type": "sphere"
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "r": 1.5,
                "type": "sphere"
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide17",
          "position": {
            "x": 10.0,
            "y": 1.499999761581421,
            "z": -6.0
          },
          "rotation": {
            "w": 1.0,
            "x": -1.1777282225011731e-06,
            "y": -6.458149987009776e-13,
            "z": -7.2546426963526756e-06
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1.0,
                      "value1": 0.0,
                      "value2": 0.0
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "r": 1.5,
                "type": "sphere"
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 15,
                "type": "dynamic"
              },
              "collider": {
                "r": 1.5,
                "type": "sphere"
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ObjectCollide18",
          "position": {
            "x": 10.0,
            "y": 1.499999761581421,
            "z": -4.0
          },
          "rotation": {
            "w": 1.0,
            "x": 8.966561608758639e-07,
            "y": -6.27790180951493e-12,
            "z": -8.086309208010789e-06
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.45,
                      "value1": 0.45,
                      "value2": 0.45
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 2.6,
                "y": 0.12,
                "z": 16
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 2.6,
                "y": 0.12,
                "z": 16
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ShootPathwayFloor",
          "position": {
            "x": 0.0,
            "y": 0.5600000023841858,
            "z": 5.0
          },
          "rotation": {
            "w": 0.7139406204223633,
            "x": 0.0,
            "y": 0.7002063393592834,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.25,
                      "value1": 0.25,
                      "value2": 0.3
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 0.25,
                "y": 2.0,
                "z": 16
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 0.25,
                "y": 2.0,
                "z": 16
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ShootPathwayLeftRail",
          "position": {
            "x": -1.149999976158142,
            "y": 1.5499999523162842,
            "z": 5.0
          },
          "rotation": {
            "w": 0.0001012137727229856,
            "x": 0.0,
            "y": 0.0,
            "z": -0.9999999403953552
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.25,
                      "value1": 0.25,
                      "value2": 0.3
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 0.25,
                "y": 2.0,
                "z": 16
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 0.25,
                "y": 2.0,
                "z": 16
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ShootPathwayRightRail",
          "position": {
            "x": 1.5,
            "y": 1.5499999523162842,
            "z": 5.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1.0,
                      "value1": 0.15,
                      "value2": 0.1
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.8,
                "y": 1.8,
                "z": 1.8
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 1.8,
                "y": 1.8,
                "z": 1.8
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ShootPathwayCubeA",
          "position": {
            "x": 0.0,
            "y": 1.5,
            "z": 2.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          },
          "type": "ShootableCube"
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1.0,
                      "value1": 0.15,
                      "value2": 0.1
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 1.8,
                "y": 1.8,
                "z": 1.8
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 1.8,
                "y": 1.8,
                "z": 1.8
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ShootPathwayCubeB",
          "position": {
            "x": 0.0,
            "y": 0.0,
            "z": 8.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          },
          "type": "ShootableCube"
        },
        {
          "components": [
            {
              "material": {
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.05,
                      "value1": 0.8,
                      "value2": 0.2
                    }
                  ]
                },
                "path": "materials/checker.mat"
              },
              "mesh": {
                "type": "box",
                "x": 2.6,
                "y": 0.2,
                "z": 2.0
              },
              "type": "MeshComponent"
            },
            {
              "body": {
                "friction": 0.5,
                "mass": 0,
                "type": "static"
              },
              "collider": {
                "type": "box",
                "x": 2.6,
                "y": 0.2,
                "z": 2.0
              },
              "type": "PhysicsComponent"
            }
          ],
          "name": "ShootPathwayExitPlatform",
          "position": {
            "x": 0.0,
            "y": 0.6499999761581421,
            "z": 13.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "components": [
            {
              "color": {
                "b": 1.0,
                "g": 1.0,
                "r": 1.0
              },
              "type": "LightComponent"
            }
          ],
          "name": "Light",
          "position": {
            "x": -2.0,
            "y": 5.0,
            "z": 2.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        }
      ],
      "name": "3DRoot",
      "position": {
        "x": 0.0,
        "y": 0.0,
        "z": 0.0
      },
      "rotation": {
        "w": 1.0,
        "x": 0.0,
        "y": 0.0,
        "z": 0.0
      },
      "scale": {
        "x": 1.0,
        "y": 1.0,
        "z": 1.0
      }
    },
    {
      "children": [
        {
          "children": [
            {
              "components": [
                {
                  "color": {
                    "a": 1.0,
                    "b": 0.0,
                    "g": 0.0,
                    "r": 0.0
                  },
                  "font": {
                    "path": "fonts/arial.ttf",
                    "size": 24
                  },
                  "text": "Play",
                  "type": "TextComponent"
                },
                {
                  "anchor": {
                    "x": 0.5,
                    "y": 0.5
                  },
                  "pivot": {
                    "x": 0.5,
                    "y": 0.5
                  },
                  "size": {
                    "x": 0.0,
                    "y": 0.0
                  },
                  "type": "RectTransformComponent"
                }
              ],
              "name": "Text",
              "position": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
              },
              "rotation": {
                "w": 1.0,
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
              },
              "scale": {
                "x": 1.0,
                "y": 1.0,
                "z": 1.0
              }
            }
          ],
          "components": [
            {
              "color": {
                "a": 1.0,
                "b": 1.0,
                "g": 1.0,
                "r": 1.0
              },
              "hovered": {
                "a": 1.0,
                "b": 0.5,
                "g": 0.5,
                "r": 0.5
              },
              "pressed": {
                "a": 1.0,
                "b": 0.20000000298023224,
                "g": 0.20000000298023224,
                "r": 0.20000000298023224
              },
              "rect": {
                "x": 150.0,
                "y": 50.0
              },
              "type": "ButtonComponent"
            },
            {
              "anchor": {
                "x": 0.5,
                "y": 0.5
              },
              "pivot": {
                "x": 0.5,
                "y": 0.5
              },
              "size": {
                "x": 150.0,
                "y": 50.0
              },
              "type": "RectTransformComponent"
            }
          ],
          "name": "PlayButton",
          "position": {
            "x": 0.0,
            "y": 100.0,
            "z": 0.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "children": [
            {
              "components": [
                {
                  "color": {
                    "a": 1.0,
                    "b": 0.0,
                    "g": 0.0,
                    "r": 0.0
                  },
                  "font": {
                    "path": "fonts/arial.ttf",
                    "size": 24
                  },
                  "text": "Edit",
                  "type": "TextComponent"
                },
                {
                  "anchor": {
                    "x": 0.5,
                    "y": 0.5
                  },
                  "pivot": {
                    "x": 0.5,
                    "y": 0.5
                  },
                  "size": {
                    "x": 0.0,
                    "y": 0.0
                  },
                  "type": "RectTransformComponent"
                }
              ],
              "name": "Editor",
              "position": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
              },
              "rotation": {
                "w": 1.0,
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
              },
              "scale": {
                "x": 1.0,
                "y": 1.0,
                "z": 1.0
              }
            }
          ],
          "components": [
            {
              "color": {
                "a": 1.0,
                "b": 1.0,
                "g": 1.0,
                "r": 1.0
              },
              "hovered": {
                "a": 1.0,
                "b": 0.5,
                "g": 0.5,
                "r": 0.5
              },
              "pressed": {
                "a": 1.0,
                "b": 0.20000000298023224,
                "g": 0.20000000298023224,
                "r": 0.20000000298023224
              },
              "rect": {
                "x": 150.0,
                "y": 50.0
              },
              "type": "ButtonComponent"
            },
            {
              "anchor": {
                "x": 0.5,
                "y": 0.5
              },
              "pivot": {
                "x": 0.5,
                "y": 0.5
              },
              "size": {
                "x": 150.0,
                "y": 50.0
              },
              "type": "RectTransformComponent"
            }
          ],
          "name": "EditButton",
          "position": {
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        },
        {
          "children": [
            {
              "components": [
                {
                  "color": {
                    "a": 1.0,
                    "b": 0.0,
                    "g": 0.0,
                    "r": 0.0
                  },
                  "font": {
                    "path": "fonts/arial.ttf",
                    "size": 24
                  },
                  "text": "Quit",
                  "type": "TextComponent"
                },
                {
                  "anchor": {
                    "x": 0.5,
                    "y": 0.5
                  },
                  "pivot": {
                    "x": 0.5,
                    "y": 0.5
                  },
                  "size": {
                    "x": 0.0,
                    "y": 0.0
                  },
                  "type": "RectTransformComponent"
                }
              ],
              "name": "Text",
              "position": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
              },
              "rotation": {
                "w": 1.0,
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
              },
              "scale": {
                "x": 1.0,
                "y": 1.0,
                "z": 1.0
              }
            }
          ],
          "components": [
            {
              "color": {
                "a": 1.0,
                "b": 1.0,
                "g": 1.0,
                "r": 1.0
              },
              "hovered": {
                "a": 1.0,
                "b": 0.5,
                "g": 0.5,
                "r": 0.5
              },
              "pressed": {
                "a": 1.0,
                "b": 0.20000000298023224,
                "g": 0.20000000298023224,
                "r": 0.20000000298023224
              },
              "rect": {
                "x": 150.0,
                "y": 50.0
              },
              "type": "ButtonComponent"
            },
            {
              "anchor": {
                "x": 0.5,
                "y": 0.5
              },
              "pivot": {
                "x": 0.5,
                "y": 0.5
              },
              "size": {
                "x": 150.0,
                "y": 50.0
              },
              "type": "RectTransformComponent"
            }
          ],
          "name": "QuitButton",
          "position": {
            "x": 0.0,
            "y": -100.0,
            "z": 0.0
          },
          "rotation": {
            "w": 1.0,
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
          },
          "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
          }
        }
      ],
      "components": [
        {
          "active": false,
          "type": "CanvasComponent"
        },
        {
          "anchor": {
            "x": 0.5,
            "y": 0.5
          },
          "pivot": {
            "x": 0.0,
            "y": 0.0
          },
          "size": {
            "x": 1280.0,
            "y": 720.0
          },
          "type": "RectTransformComponent"
        }
      ],
      "name": "MainCanvas",
      "position": {
        "x": 0.0,
        "y": 0.0,
        "z": 0.0
      },
      "rotation": {
        "w": 1.0,
        "x": 0.0,
        "y": 0.0,
        "z": 0.0
      },
      "scale": {
        "x": 1.0,
        "y": 1.0,
        "z": 1.0
      }
    }
  ]
}