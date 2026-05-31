{
  "name": "MyScene",
  "objects": [
    {
      "name": "3DRoot",
      "children": [
        {
          "name": "MainPlayer",
          "type": "Player",
          "position": {
            "x": 0,
            "y": 2,
            "z": -7
          },
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
              "type": "AudioComponent",
              "volume": 1.0,
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
              ]
            }
          ],
          "children": [
            {
              "name": "Gun",
              "type": "gltf",
              "path": "models/CarbineGun/scene.gltf",
              "position": {
                "x": 0.75,
                "y": -0.5,
                "z": -0.75
              },
              "scale": {
                "x": -1.0,
                "y": 1.0,
                "z": 1.0
              }
            }
          ]
        },
        {
          "name": "GunPickup",
          "type": "GunPickup",
          "position": {
            "x": 0,
            "y": 1.2,
            "z": -3
          },
          "rotationSpeed": 90.0,
          "components": [
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "sphere",
                "x": 0.75
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ],
          "children": [
            {
              "name": "PickupGunModel",
              "type": "gltf",
              "path": "models/CarbineGun/scene.gltf",
              "rotation": {
              "x":0.0,
              "y":0.5,
              "z":0.0,
              "w":1.0
              },
              "scale": {
                "x": 1.0,
                "y": 1.0,
                "z": 1.0
              }
            }
          ]
        },
        {
          "name": "Ground",
          "position": {
            "x": 0,
            "y": 0,
            "z": 0
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 30,
                "y": 1,
                "z": 30
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 30,
                "y": 1,
                "z": 30
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "LeftWall",
          "position": {
            "x": -15.5,
            "y": 3,
            "z": 0
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1,
                "y": 5,
                "z": 30
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1,
                "y": 5,
                "z": 30
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "RightWall",
          "position": {
            "x": 15.5,
            "y": 3,
            "z": 0
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1,
                "y": 5,
                "z": 30
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1,
                "y": 5,
                "z": 30
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "FrontWall",
          "position": {
            "x": 0,
            "y": 3,
            "z": -15.5
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 30,
                "y": 5,
                "z": 1
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 30,
                "y": 5,
                "z": 1
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "BackWall",
          "position": {
            "x": 0,
            "y": 3,
            "z": 15.5
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 30,
                "y": 5,
                "z": 1
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 30,
                "y": 5,
                "z": 1
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "PlatformA",
          "position": {
            "x": -14,
            "y": 1.05,
            "z": -14
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 2,
                "y": 1.1,
                "z": 2
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 2,
                "y": 1.1,
                "z": 2
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "LadderA_1",
          "position": {
            "x": -12.85,
            "y": 1.35,
            "z": -14
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "LadderA_2",
          "position": {
            "x": -12.55,
            "y": 1.25,
            "z": -14
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "LadderA_3",
          "position": {
            "x": -12.25,
            "y": 1.15,
            "z": -14
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "LadderA_4",
          "position": {
            "x": -11.95,
            "y": 1.05,
            "z": -14
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "LadderA_5",
          "position": {
            "x": -11.65,
            "y": 0.95,
            "z": -14
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "LadderA_6",
          "position": {
            "x": -11.35,
            "y": 0.85,
            "z": -14
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "LadderA_7",
          "position": {
            "x": -11.05,
            "y": 0.75,
            "z": -14
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "LadderA_8",
          "position": {
            "x": -10.75,
            "y": 0.65,
            "z": -14
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "LadderA_9",
          "position": {
            "x": -10.45,
            "y": 0.55,
            "z": -14
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 0.3,
                "y": 0.1,
                "z": 2
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "PlatformB",
          "position": {
            "x": -14,
            "y": 1.05,
            "z": -10
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 2,
                "y": 1.1,
                "z": 2
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 2,
                "y": 1.1,
                "z": 2
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "PlatformС",
          "position": {
            "x": 10,
            "y": 1.05,
            "z": 1.5
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 2,
                "y": 1.1,
                "z": 14
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 2,
                "y": 1.1,
                "z": 14
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "PlatformD",
          "position": {
            "x": -7,
            "y": 1.05,
            "z": -3
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 8,
                "y": 1.1,
                "z": 2
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 8,
                "y": 1.1,
                "z": 2
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "InternalWall1",
          "position": {
            "x": -1,
            "y": 1.5,
            "z": -11
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1,
                "y": 2,
                "z": 8
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1,
                "y": 2,
                "z": 8
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "InternalWall2",
          "position": {
            "x": 1.5,
            "y": 1.5,
            "z": -1
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1,
                "y": 2,
                "z": 8
              },
              "material": {
                "path": "materials/checker.mat"
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1,
                "y": 2,
                "z": 8
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "JumpPlatform",
          "type": "JumpPlatform",
          "position": {
            "x": -7,
            "y": 1.0,
            "z": 1
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 2,
                "y": 0.2,
                "z": 2
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1.0,
                      "value1": 0.0,
                      "value2": 0.0
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 2,
                "y": 0.2,
                "z": 2
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide1",
          "type": "ShootableCube",
          "position": {
            "x": 7.75,
            "y": 1.251,
            "z": -5
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.0,
                      "value1": 1.0,
                      "value2": 0.0
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide2",
          "type": "ShootableCube",
          "position": {
            "x": 12.25,
            "y": 1.251,
            "z": -4
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.7,
                      "value1": 0.5,
                      "value2": 0.2
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide3",
          "type": "ShootableCube",
          "position": {
            "x": 7.75,
            "y": 1.251,
            "z": -3
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.2,
                      "value1": 0.5,
                      "value2": 0.1
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide4",
          "type": "ShootableCube",
          "position": {
            "x": 12.25,
            "y": 1.251,
            "z": -2
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.2,
                      "value1": 0.1,
                      "value2": 0.9
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide5",
          "type": "ShootableCube",
          "position": {
            "x": 7.75,
            "y": 1.251,
            "z": -1
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1,
                      "value1": 0,
                      "value2": 1
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide6",
          "type": "ShootableCube",
          "position": {
            "x": 12.25,
            "y": 1.251,
            "z": 0
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.1,
                      "value1": 0.8,
                      "value2": 0.7
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide7",
          "type": "ShootableCube",
          "position": {
            "x": 7.75,
            "y": 1.251,
            "z": 1
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.1,
                      "value1": 0.8,
                      "value2": 0.7
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide8",
          "type": "ShootableCube",
          "position": {
            "x": 12.25,
            "y": 1.251,
            "z": 2
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.9,
                      "value1": 0.2,
                      "value2": 0.3
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide9",
          "type": "ShootableCube",
          "position": {
            "x": 7.75,
            "y": 1.251,
            "z": 3
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.2,
                      "value1": 0.2,
                      "value2": 0.3
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide10",
          "type": "ShootableCube",
          "position": {
            "x": 12.25,
            "y": 1.251,
            "z": 4
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.5,
                      "value1": 0.1,
                      "value2": 0.9
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide11",
          "type": "ShootableCube",
          "position": {
            "x": 7.75,
            "y": 1.251,
            "z": 5
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1,
                      "value1": 1,
                      "value2": 0
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide12",
          "type": "ShootableCube",
          "position": {
            "x": 12.25,
            "y": 1.251,
            "z": 6
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1,
                      "value1": 1,
                      "value2": 0
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide13",
          "type": "ShootableCube",
          "position": {
            "x": 7.75,
            "y": 1.251,
            "z": 7
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1,
                      "value1": 0,
                      "value2": 0
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide14",
          "type": "ShootableCube",
          "position": {
            "x": 12.25,
            "y": 1.251,
            "z": 8
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0,
                      "value1": 0,
                      "value2": 1
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide15",
          "type": "ShootableCube",
          "position": {
            "x": 12.25,
            "y": 2.752,
            "z": 2
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0,
                      "value1": 1,
                      "value2": 1
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 1.5,
                "y": 1.5,
                "z": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "JumpPlatform",
          "type": "JumpPlatform",
          "position": {
            "x": 0,
            "y": 1.0,
            "z": 1
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "box",
                "x": 2,
                "y": 0.2,
                "z": 2
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1,
                      "value1": 0,
                      "value2": 0
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "box",
                "x": 2,
                "y": 0.2,
                "z": 2
              },
              "body": {
                "mass": 0,
                "friction": 0.5,
                "type": "static"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide16",
          "position": {
            "x": 10,
            "y": 1.251,
            "z": -8
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "sphere",
                "r": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.0,
                      "value1": 1.0,
                      "value2": 0.0
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "sphere",
                "r": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide17",
          "position": {
            "x": 10,
            "y": 1.251,
            "z": -6
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "sphere",
                "r": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 0.0,
                      "value1": 0.0,
                      "value2": 1.0
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "sphere",
                "r": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "ObjectCollide18",
          "position": {
            "x": 10,
            "y": 1.251,
            "z": -4
          },
          "components": [
            {
              "type": "MeshComponent",
              "mesh": {
                "type": "sphere",
                "r": 1.5
              },
              "material": {
                "path": "materials/checker.mat",
                "params": {
                  "float3": [
                    {
                      "name": "color",
                      "value0": 1.0,
                      "value1": 0.0,
                      "value2": 0.0
                    }
                  ]
                }
              }
            },
            {
              "type": "PhysicsComponent",
              "collider": {
                "type": "sphere",
                "r": 1.5
              },
              "body": {
                "mass": 15,
                "friction": 0.5,
                "type": "dynamic"
              }
            }
          ]
        },
        {
          "name": "Light",
          "position": {
            "x": -2,
            "y": 5,
            "z": 2
          },
          "components": [
            {
              "type": "LightComponent",
              "color": {
                "r": 1,
                "g": 1,
                "b": 1
              }
            }
          ]
        }
      ]
    },
    {
      "name": "MainCanvas",
      "components": [
        {
          "type": "CanvasComponent"
        },
        {
          "type": "RectTransformComponent",
          "anchor": {
            "x": 0.5,
            "y": 0.5
          },
          "pivot": {
            "x": 0,
            "y": 0
          }
        }
      ],
      "children": [
        {
          "name": "PlayButton",
          "position": {
            "x": 0,
            "y": 100,
            "z": 0
          },
          "components": [
            {
              "type": "ButtonComponent"
            },
            {
              "type": "RectTransformComponent",
              "anchor": {
                "x": 0.5,
                "y": 0.5
              },
              "pivot": {
                "x": 0.5,
                "y": 0.5
              },
              "size": {
                "x": 150,
                "y": 50
              }
            }
          ],
          "children": [
            {
              "name": "Text",
              "components": [
                {
                  "type": "TextComponent",
                  "text": "Play",
                  "font": {
                    "path": "fonts/arial.ttf",
                    "size": 24
                  },
                  "color": {
                    "r": 0,
                    "g": 0,
                    "b": 0,
                    "a": 1
                  }
                },
                {
                  "type": "RectTransformComponent",
                  "anchor": {
                    "x": 0.5,
                    "y": 0.5
                  },
                  "pivot": {
                    "x": 0.5,
                    "y": 0.5
                  }
                }
              ]
            }
          ]
        },
        {
          "name": "EditButton",
          "position": {
            "x": 0,
            "y": 0,
            "z": 0
          },
          "components": [
            {
              "type": "ButtonComponent"
            },
            {
              "type": "RectTransformComponent",
              "anchor": {
                "x": 0.5,
                "y": 0.5
              },
              "pivot": {
                "x": 0.5,
                "y": 0.5
              },
              "size": {
                "x": 150,
                "y": 50
              }
            }
          ],
          "children": [
            {
              "name": "Editor",
              "components": [
                {
                  "type": "TextComponent",
                  "text": "Edit",
                  "font": {
                    "path": "fonts/arial.ttf",
                    "size": 24
                  },
                  "color": {
                    "r": 0,
                    "g": 0,
                    "b": 0,
                    "a": 1
                  }
                },
                {
                  "type": "RectTransformComponent",
                  "anchor": {
                    "x": 0.5,
                    "y": 0.5
                  },
                  "pivot": {
                    "x": 0.5,
                    "y": 0.5
                  }
                }
              ]
            }
          ]
        },
        {
          "name": "QuitButton",
          "position": {
            "x": 0,
            "y": -100,
            "z": 0
          },
          "components": [
            {
              "type": "ButtonComponent"
            },
            {
              "type": "RectTransformComponent",
              "anchor": {
                "x": 0.5,
                "y": 0.5
              },
              "pivot": {
                "x": 0.5,
                "y": 0.5
              },
              "size": {
                "x": 150,
                "y": 50
              }
            }
          ],
          "children": [
            {
              "name": "Text",
              "components": [
                {
                  "type": "TextComponent",
                  "text": "Quit",
                  "font": {
                    "path": "fonts/arial.ttf",
                    "size": 24
                  },
                  "color": {
                    "r": 0,
                    "g": 0,
                    "b": 0,
                    "a": 1
                  }
                },
                {
                  "type": "RectTransformComponent",
                  "anchor": {
                    "x": 0.5,
                    "y": 0.5
                  },
                  "pivot": {
                    "x": 0.5,
                    "y": 0.5
                  }
                }
              ]
            }
          ]
        }
      ]
    }
  ],
  "camera": "MainPlayer",
  "activeCanvas": "MainCanvas"
}