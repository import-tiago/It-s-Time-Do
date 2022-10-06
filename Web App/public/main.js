/**
 * Copyright 2016 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
"use strict";
const listModes = [
  { id: 1, mode: "Normal" },
  { id: 2, mode: "Brancas" },
  { id: 3, mode: "Escuras" },
  { id: 4, mode: "Coloridas" },
  { id: 5, mode: "Cama e Banho" },
  { id: 6, mode: "Edredon" },
  { id: 7, mode: "Tênis" },
  { id: 8, mode: "Rápido" },
  { id: 9, mode: "Delicado" },
  { id: 10, mode: "Pesada/Jeans" },
  { id: 11, mode: "Tira-manchas" },
  { id: 12, mode: "Limpeza de cesto" },
];
// Initializes the Demo.
function Demo() {
  document.addEventListener(
    "DOMContentLoaded",
    function () {
      this.logged = document.getElementById("loggedUser");
      this.loginCard = document.getElementById("mycard");
      this.selectedMode = document.getElementById("selectMode");
      this.signOutButton = document.getElementById("signOutItem");
      this.signInItem = document.getElementById("signInItem");
      this.shedulleItem = document.getElementById("shedulleItem");

      this.SheduleButton = document.getElementById("shedulleButton");

      this.ListWashes = document.getElementById("listWashes");

      this.signInButton = document.getElementById("sigInButton");

      this.nameContainer = document.getElementById("demo-name-container");
      this.fcmErrorContainer = document.getElementById(
        "demo-fcm-error-container"
      );
      this.deleteButton = document.getElementById("demo-delete-button");
      this.signedOutCard = document.getElementById("demo-signed-out-card");
      this.signedInCard = document.getElementById("demo-signed-in-card");
      this.usersContainer = document.getElementById("demo-all-users-list");
      this.usersCard = document.getElementById("demo-all-users-card");
      this.snackbar = document.getElementById("demo-snackbar");

      // Bind events.
      this.SheduleButton.addEventListener("click", this.sheduleEvent.bind());
      this.signInButton.addEventListener("click", this.signIn.bind(this));
      this.signOutButton.addEventListener("click", this.signOut.bind(this));
      listModes.forEach((element) => {
        let child = document.createElement("option");
        child.innerHTML = element.mode;
        child.setAttribute("id", element.id);
        this.selectedMode.appendChild(child);
      });

      firebase.auth().onAuthStateChanged(this.onAuthStateChanged.bind(this));
      firebase.messaging().onMessage(this.onMessage.bind(this));
    }.bind(this)
  );
}

// Triggered on Firebase auth state change.
Demo.prototype.onAuthStateChanged = function (user) {
  // If this is just an ID token refresh we exit.
  if (user && this.currentUid === user.uid) {
    return;
  }
  // Remove all Firebase realtime database listeners.
  if (this.listeners) {
    this.listeners.forEach(function (ref) {
      ref.off();
    });
  }
  this.listeners = [];

  // Adjust UI depending on user state.
  if (user) {
    this.saveToken();
    this.getWashingMachine();
    //this.displayAllUsers();
    this.loginCard.style.display = "none";
    this.signInItem.classList.remove("active");
    this.shedulleItem.classList.add("active");
    this.signOutButton.style.display = "block";
    this.signInItem.style.display = "none";
    this.logged.style.display = "block";
    window.alert(user);
  } else {
    this.loginCard.style.display = "flex";
    this.signInItem.classList.add("active");
    this.shedulleItem.classList.remove("active");
    this.signOutButton.style.display = "none";
    this.signInItem.style.display = "block";
    this.logged.style.display = "none";
    this.currentUid = null;
  }
};

// Initiates the sign-in flow using LinkedIn sign in in a popup.
Demo.prototype.signIn = function () {
  let errorMessage = "Erro interno no servidor";
  try {
    const signInEmail = document.getElementById("floatingInput").value;
    const signInPassword = document.getElementById("floatingPassword").value;
    let user = firebase
      .auth()
      .signInWithEmailAndPassword(signInEmail, signInPassword);
    if (user.error) {
      switch (user.error.code) {
        case "auth/invalid-email":
          errorMessage = "Email inválido!";
          break;
        case "auth/invalid-password":
          errorMessage = "Email inválido!";
          break;
      }
      window.alert(errorMessage);
    }
  } catch {
    window.alert(errorMessage);
  }
};

// Signs-out of Firebase.
Demo.prototype.signOut = function () {
  firebase.auth().signOut();
};

// Deletes the user's account.
Demo.prototype.deleteAccount = function () {
  return firebase
    .database()
    .ref("users/" + this.currentUid)
    .remove()
    .then(function () {
      return firebase
        .auth()
        .currentUser.delete()
        .then(function () {
          window.alert("Account deleted");
        })
        .catch(function (error) {
          if (error.code === "auth/requires-recent-login") {
            window.alert(
              "You need to have recently signed-in to delete your account. Please sign-in and try again."
            );
            firebase.auth().signOut();
          }
        });
    });
};

// This is called when a notification is received while the app is in focus.
// When the app is not in focus or if the tab is closed, this function is not called and the FCM notifications is
// handled by the Service worker which displays a browser popup notification if the browser supports it.
Demo.prototype.onMessage = function (payload) {
  console.log("Notifications received.", payload);

  // Normally our Function sends a notification payload, we check just in case.
  if (payload.notification) {
    // If notifications are supported on this browser we display one.
    // Note: This is for demo purposes only. For a good user experience it is not recommended to display browser
    // notifications while the app is in focus. In a production app you probably want to only display some form of
    // in-app notifications like the snackbar (see below).
    if (window.Notification instanceof Function) {
      // This displays a notification if notifications have been granted.
      new Notification(payload.notification.title, payload.notification);
    }
    // Display the notification content in the Snackbar too.
    this.snackbar.MaterialSnackbar.showSnackbar({
      message: payload.notification.body,
    });
  }
};

// Saves the token to the database if available. If not request permissions.
Demo.prototype.saveToken = function () {
  firebase
    .messaging()
    .getToken()
    .then(
      function (currentToken) {
        if (currentToken) {
          firebase
            .database()
            .ref("Notification_Tokens/" + currentToken)
            .set(true);
        } else {
          this.requestPermission();
        }
      }.bind(this)
    )
    .catch(
      function (err) {
        console.error("Unable to get messaging token.", err);
        if (err.code === "messaging/permission-default") {
          this.fcmErrorContainer.innerText =
            "You have not enabled notifications on this browser. To enable notifications reload the page and allow notifications using the permission dialog.";
        } else if (err.code === "messaging/notifications-blocked") {
          this.fcmErrorContainer.innerHTML =
            'You have blocked notifications on this browser. To enable notifications follow these instructions: <a href="https://support.google.com/chrome/answer/114662?visit_id=1-636150657126357237-2267048771&rd=1&co=GENIE.Platform%3DAndroid&oco=1">Android Chrome Instructions</a><a href="https://support.google.com/chrome/answer/6148059">Desktop Chrome Instructions</a>';
        }
      }.bind(this)
    );
};

// Requests permission to send notifications on this browser.
Demo.prototype.requestPermission = function () {
  console.log("Requesting permission...");
  firebase
    .messaging()
    .requestPermission()
    .then(
      function () {
        console.log("Notification permission granted.");
        this.saveToken();
      }.bind(this)
    )
    .catch(function (err) {
      console.error("Unable to get permission to notify.", err);
    });
};

Demo.prototype.getWashingMachine = function () {
  var washes = firebase.database().ref("Washing_Machine/Last_Task");
  washes.on("value", (snapshot) => {
    const data = snapshot.val();
    this.updateListWashes(data);
  });
};
Demo.prototype.updateListWashes = function (data) {
  const dates = Object.keys(data);
  if (!dates) {
    return;
  }
  var i = 0;
  for (let date of dates) {
    const Hours = Object.keys(data[date]);
    if (Hours) {
      for (let Hour of Hours) {
        const WashPrams = {
          ...data[date][Hour],
        };
        // console.log(date);
        // console.log(Hour);
        // console.log(WashPrams.Duration);
        // console.log( WashPrams.Finish );
        let template = `
       
        <span style="font-size: 3em; color: #00D100;" class="material-symbols-outlined">task_alt</span>
        <div class="d-flex gap-2 w-100 justify-content-between ">
          <div class="d-flex-column align-items-center">
            <p class="mb-0 opacity-75">${date}</p>
            <div class="d-flex">
              <h6 style="margin-top: 0;" class="mb-0">Duração: </h6>
              <p  style="margin-top: 0; font-size: large; font-weight: bold;"  >${WashPrams.Duration}</p>
            </div>
          </div>
          <div class="d-flex-column">
            <div class="d-flex">
              <small class="opacity-50 text-nowrap">Das</small>
              <small class="opacity-50 text-nowrap" style="margin-left:0.5em;">${Hour}</small>
            </div>
            <div class="d-flex">
              <small class="opacity-50 text-nowrap">às</small>
              <small class="opacity-50 text-nowrap" style="margin-left:0.5em;">${WashPrams.Finish}</small>
            </div>
          </div>
          </div>
      
        `;
        // <div href="#" class="list-group-item list-group-item-action d-flex align-items-center gap-3 py-3" aria-current="true">
        var div = document.createElement("div");
        div.classList.add(
          "list-group-item",
          "list-group-item-action",
          "d-flex",
          "align-items-center",
          "gap-3",
          "py-3"
        );
        div.innerHTML = template;

        this.ListWashes.appendChild(div);
        if (i++ >= 10) break;
      }
    }
  }
};

Demo.prototype.sheduleEvent = function () {
  var time = document.getElementById("timeShedule").value;
  console.log("agendarmento click");
  console.log(time);

  var updates = {};
  updates["/IoT_Device/Schedule"] = time;
  firebase.database().ref().update(updates);
};
window.demo = new Demo();
