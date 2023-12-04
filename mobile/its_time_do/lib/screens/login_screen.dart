import 'dart:async';
import 'package:its_time_do/stores/mqtt_store.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:get_it/get_it.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'events_screen.dart';
import 'package:flutter_mobx/flutter_mobx.dart';
import 'package:rounded_loading_button/rounded_loading_button.dart';
import 'package:flutter/services.dart';
import 'package:yaml/yaml.dart';

class LoginScreen extends StatefulWidget {
  const LoginScreen({super.key});

  @override
  State<LoginScreen> createState() => _LoginScreenState();
}

class _LoginScreenState extends State<LoginScreen> {
  final controller = GetIt.I.get<MqttController>();
  final TextEditingController _userAuthValue = TextEditingController();
  final TextEditingController _passwordAuthValue = TextEditingController();
  final TextEditingController _brokerAddrController = TextEditingController();

  String lastBroker = '';

  void _loadNVS() async {
    try {
      SharedPreferences prefs = await SharedPreferences.getInstance();
      var user = prefs.getString("user") ?? '';
      var password = prefs.getString("password") ?? '';
      var host = prefs.getString("host") ?? 'mqtt.tago.io';
      var remeberMe = prefs.getBool("remember_me") ?? false;

      _brokerAddrController.text = host;
      controller.host = host;
      lastBroker = _brokerAddrController.text;

      if (remeberMe) {
        controller.toggleRemebermMe(true);
        _userAuthValue.text = user;
        _passwordAuthValue.text = password;
      }
    } catch (e) {
      if (kDebugMode) {
        print(e);
      }
    }
  }

  @override
  void initState() {
    super.initState();
    _loadNVS();
    _brokerAddrController.text = controller.host;
    lastBroker = _brokerAddrController.text;
  }

  final RoundedLoadingButtonController _loginController = RoundedLoadingButtonController();

  @override
  void dispose() {
    _userAuthValue.dispose();
    _passwordAuthValue.dispose();
    super.dispose();
  }

  Future<bool> brokerConnect() async {
    controller.user = '';
    controller.password = '';
    controller.user = _userAuthValue.text;
    controller.password = _passwordAuthValue.text;
    if (await controller.connect()) {
      return true;
    } else {
      return false;
    }
  }

  final buttonColor = const Color.fromARGB(255, 246, 172, 44);

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: () {
        FocusScopeNode currentFocus = FocusScope.of(context);
        if (!currentFocus.hasPrimaryFocus) {
          currentFocus.unfocus();
        }
      },
      child: PopScope(
        canPop: false,
        child: Scaffold(
          resizeToAvoidBottomInset: false,
          appBar: AppBar(
            actions: <Widget>[
              IconButton(
                icon: const Icon(Icons.settings),
                tooltip: 'CONFIGURAÇÕES',
                onPressed: () {
                  showDialog(
                      context: context,
                      builder: (context) {
                        return AlertDialog(
                          title: const Text('MQTT Broker Address'),
                          content: TextField(
                            onChanged: (value) {},
                            controller: _brokerAddrController,
                            style: const TextStyle(fontSize: 22.0),
                          ),
                          actions: <Widget>[
                            TextButton(
                                child: const Text(
                                  'cancel',
                                  style: TextStyle(fontSize: 18.0),
                                ),
                                onPressed: () {
                                  _brokerAddrController.text = lastBroker;
                                  Navigator.of(context).popUntil((route) => route.isFirst);
                                }),
                            TextButton(
                              child: const Text(
                                'save',
                                style: TextStyle(fontSize: 18.0),
                              ),
                              onPressed: () {
                                lastBroker = _brokerAddrController.text;
                                SharedPreferences.getInstance().then(
                                  (prefs) {
                                    prefs.setString('host', _brokerAddrController.text);
                                  },
                                );
                                controller.host = _brokerAddrController.text;
                                lastBroker = _brokerAddrController.text;
                                Navigator.of(context).popUntil((route) => route.isFirst);
                              },
                            ),
                          ],
                        );
                      });
                },
              ),
            ],
          ),
          body: Stack(children: <Widget>[
            Positioned(
              child: SingleChildScrollView(
                child: Column(
                  children: <Widget>[
                    const Padding(
                      padding: EdgeInsets.only(top: 60.0),
                      child: Center(
                        child: SizedBox(
                            width: 200,
                            height: 150,
                            child: Image(
                              image: AssetImage('lib/assets/logo.png'),
                              width: 70,
                            )),
                      ),
                    ),
                    const SizedBox(
                      height: 20,
                    ),
                    Padding(
                      padding: const EdgeInsets.only(left: 15.0, right: 15.0, top: 15, bottom: 0),
                      child: TextFormField(
                        controller: _userAuthValue,
                        keyboardType: TextInputType.name,
                        autocorrect: false,
                        enableSuggestions: false,
                        decoration: const InputDecoration(
                            labelText: 'user',
                            hintText: 'insert your MQTT client user',
                            floatingLabelStyle: TextStyle(
                              fontSize: 20,
                            ),
                            hintStyle: TextStyle(fontSize: 16)),
                        style: const TextStyle(
                          fontSize: 20.0,
                        ),
                      ),
                    ),
                    Padding(
                      padding: const EdgeInsets.only(left: 15.0, right: 15.0, top: 15, bottom: 0),
                      child: TextFormField(
                        controller: _passwordAuthValue,
                        keyboardType: TextInputType.visiblePassword,
                        autocorrect: false,
                        enableSuggestions: false,
                        obscureText: true,
                        decoration: const InputDecoration(
                            labelText: 'password',
                            hintText: 'insert your MQTT client password',
                            floatingLabelStyle: TextStyle(
                              fontSize: 20,
                            ),
                            hintStyle: TextStyle(fontSize: 16)),
                        style: const TextStyle(fontSize: 20.0),
                      ),
                    ),
                    const SizedBox(
                      height: 10,
                    ),
                    Observer(builder: (_) {
                      return Row(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: [
                          Checkbox(
                              value: controller.isCheckedRememberMe,
                              onChanged: (bool? value) {
                                controller.toggleRemebermMe(value!);
                                SharedPreferences.getInstance().then(
                                  (prefs) {
                                    prefs.setBool("remember_me", value);
                                    prefs.setString('user', _userAuthValue.text);
                                    prefs.setString('password', _passwordAuthValue.text);
                                  },
                                );
                              }),
                          const Text(
                            "Remember",
                            style: TextStyle(fontSize: 20),
                          )
                        ],
                      );
                    }),
                    const SizedBox(
                      height: 10,
                    ),
                    RoundedLoadingButton(
                      color: buttonColor,
                      failedIcon: Icons.cottage,
                      controller: _loginController,
                      onPressed: () {
                        controller.authResult = '';
                        brokerConnect().then((value) {
                          _loginController.reset();
                          if (value) {
                            Navigator.push(context, MaterialPageRoute(builder: (context) => const EventsScreen()));
                          }
                        });
                      },
                      child: const Text(
                        'Sign in',
                        style: TextStyle(fontSize: 25),
                      ),
                    ),
                    const SizedBox(
                      height: 15,
                    ),
                    Observer(
                      builder: (_) => Text(
                        controller.authResult,
                        style: const TextStyle(color: Color.fromARGB(201, 218, 20, 20), fontSize: 16),
                      ),
                    ),
                  ],
                ),
              ),
            ),
            Positioned(
              child: Align(
                alignment: Alignment.bottomCenter,
                child: Padding(
                  padding: const EdgeInsets.only(bottom: 5),
                  child: FutureBuilder(
                      future: rootBundle.loadString("pubspec.yaml"),
                      builder: (context, snapshot) {
                        String version = ' unknown';
                        if (snapshot.hasData) {
                          var yaml = loadYaml(snapshot.data.toString());
                          version = yaml["version"];
                        }
                        return Text(
                          'release v$version',
                          style: const TextStyle(color: Colors.black26, fontSize: 16),
                        );
                      }),
                ),
              ),
            )
          ]),
        ),
      ),
    );
  }
}
