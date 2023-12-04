import 'package:its_time_do/stores/mqtt_store.dart';
import 'package:flutter/material.dart';
import 'package:flutter_mobx/flutter_mobx.dart';
import 'package:get_it/get_it.dart';
import 'package:omni_datetime_picker/omni_datetime_picker.dart';

class EventsScreen extends StatefulWidget {
  const EventsScreen({super.key});

  @override
  State<EventsScreen> createState() => _EventsScreenState();
}

class _EventsScreenState extends State<EventsScreen> {
  final controller = GetIt.I.get<MqttController>();

  void _showExitConfirmationDialog(BuildContext context) {
    showDialog(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          title: const Text('Sign out'),
          content: const Text(
            'Do you really want to disconnect?',
            style: TextStyle(fontSize: 18.0),
          ),
          actions: <Widget>[
            TextButton(
                child: const Text(
                  'cancel',
                  style: TextStyle(fontSize: 18.0),
                ),
                onPressed: () {
                  Navigator.of(context).pop();
                }),
            TextButton(
              child: const Text(
                'yes',
                style: TextStyle(fontSize: 18.0),
              ),
              onPressed: () {
                Navigator.of(context).pop();
                _logout();
                controller.eventsQueueJson.clear();
              },
            ),
          ],
        );
      },
    );
  }

  void _logout() {
    controller.disconnect();
    Navigator.pop(context);
  }

  @override
  Widget build(BuildContext context) {
    return PopScope(
      canPop: false,
      child: Scaffold(
        appBar: AppBar(
          automaticallyImplyLeading: false,
          title: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [Text(controller.user.toUpperCase()), const Text('EVENTOS')]),
          actions: <Widget>[
            IconButton(
              icon: const Icon(Icons.exit_to_app),
              onPressed: () {
                _showExitConfirmationDialog(context);
              },
            ),
          ],
        ),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.start,
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              ElevatedButton(
                onPressed: () async {
                  DateTime? dateTime = await showOmniDateTimePicker(
                    context: context,
                    initialDate: DateTime.now(),
                    firstDate: DateTime(1600).subtract(const Duration(days: 3652)),
                    lastDate: DateTime.now().add(
                      const Duration(days: 3652),
                    ),
                    is24HourMode: true,
                  );

                  print("dateTime: $dateTime");
                },
                child: const Text("Show DateTime Picker"),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
