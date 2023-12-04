/*
v1.0
- initial release.
*/

import 'package:flutter/material.dart';
import 'package:get_it/get_it.dart';
import 'screens/login_screen.dart';
import 'package:google_fonts/google_fonts.dart';
import 'stores/mqtt_store.dart';

void main() {
  final getIt = GetIt.instance;

  getIt.registerSingleton<MqttController>(MqttController());

  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  final primaryColor = const Color.fromARGB(255, 0, 0, 0);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      theme: ThemeData(
        brightness: Brightness.light,
        useMaterial3: true,
        colorScheme: ColorScheme.fromSeed(
          seedColor: Colors.red,
          primary: const Color.fromARGB(161, 0, 0, 0),
          secondary: const Color.fromARGB(255, 251, 172, 36),
          background: Colors.white,
          surfaceTint: Colors.transparent,
        ),
        textTheme: GoogleFonts.montserratTextTheme(),
      ),
      debugShowCheckedModeBanner: false,
      home: const LoginScreen(),
    );
  }
}
