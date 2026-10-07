console.log("Student Database Dashboard Loaded");

const API_URL = "http://127.0.0.1:8081";

let currentUsername = "";
let currentPassword = "";

let studentRecords = [];
let recentActivities = [];


/* =========================================================
   ELEMENTS
========================================================= */

const authSection = document.getElementById("authSection");
const dashboardSection = document.getElementById("dashboardSection");

const usernameInput = document.getElementById("username");
const passwordInput = document.getElementById("password");

const loginBtn = document.getElementById("loginBtn");
const registerBtn = document.getElementById("registerBtn");

const authMessage = document.getElementById("authMessage");


/* =========================================================
   SERVER COMMAND
========================================================= */

async function sendCommand(command) {

    try {

        const response = await fetch(
            `${API_URL}/?command=${encodeURIComponent(command)}` +
            `&username=${encodeURIComponent(currentUsername)}` +
            `&password=${encodeURIComponent(currentPassword)}`
        );

        return await response.text();

    } catch (error) {

        console.error("Server Error:", error);

        return "ERROR: Cannot connect to dashboard API.";
    }
}


/* =========================================================
   LOGIN
========================================================= */

if (loginBtn) {

    loginBtn.addEventListener("click", async function () {

        const username = usernameInput.value.trim();
        const password = passwordInput.value.trim();

        if (!username || !password) {

            authMessage.textContent =
                "Please enter username and password.";

            return;
        }

        try {

            const response = await fetch(
                `${API_URL}/?command=LOGIN` +
                `&username=${encodeURIComponent(username)}` +
                `&password=${encodeURIComponent(password)}`
            );

            const result = await response.text();

            console.log("Login:", result);

            authMessage.textContent = result;

            if (result.includes("SUCCESS")) {

                currentUsername = username;
                currentPassword = password;

                authSection.style.display = "none";
                dashboardSection.style.display = "block";

                addActivity(
                    "User logged in",
                    `Welcome ${username}`,
                    "green"
                );

                await displayRecords();
                await loadSystemHealth();
            }

        } catch (error) {

            console.error("Login Error:", error);

            authMessage.textContent =
                "Cannot connect to server. Make sure the server and dashboard API are running.";
        }

    });

}


/* =========================================================
   REGISTER
========================================================= */

if (registerBtn) {

    registerBtn.addEventListener("click", async function () {

        const username = usernameInput.value.trim();
        const password = passwordInput.value.trim();

        if (!username || !password) {

            authMessage.textContent =
                "Please enter username and password.";

            return;
        }

        try {

            const response = await fetch(
                `${API_URL}/?command=REGISTER` +
                `&username=${encodeURIComponent(username)}` +
                `&password=${encodeURIComponent(password)}`
            );

            const result = await response.text();

            console.log("Register:", result);

            authMessage.textContent = result;

        } catch (error) {

            console.error("Register Error:", error);

            authMessage.textContent =
                "Cannot connect to server.";
        }

    });

}


/* =========================================================
   LOGOUT
========================================================= */

const logoutBtn = document.getElementById("logoutBtn");

if (logoutBtn) {

    logoutBtn.addEventListener("click", async function () {

        await sendCommand("LOGOUT");

        currentUsername = "";
        currentPassword = "";

        usernameInput.value = "";
        passwordInput.value = "";

        dashboardSection.style.display = "none";
        authSection.style.display = "flex";

        authMessage.textContent =
            "Logged out successfully.";

    });

}


/* =========================================================
   DISPLAY RECORDS
========================================================= */

const displayBtn = document.getElementById("displayBtn");

if (displayBtn) {

    displayBtn.addEventListener("click", async function () {

        await displayRecords();

    });

}


async function displayRecords() {

    const result = await sendCommand("DISPLAY");

    console.log("Display:", result);

    parseStudentRecords(result);

    displayStudents(result);

    updateAnalytics();

    addActivity(
        "Student records loaded",
        `${studentRecords.length} records retrieved`,
        "blue"
    );
}


/* =========================================================
   PARSE STUDENT DATA
========================================================= */

function parseStudentRecords(result) {

    studentRecords = [];

    const lines = result.split("\n");

    lines.forEach(function (line) {

        line = line.trim();

        if (!line.startsWith("ID:"))
            return;

        const parts = line.split("|");

        if (parts.length < 5)
            return;

        const id =
            parts[0].replace("ID:", "").trim();

        const name =
            parts[1].replace("Name:", "").trim();

        const course =
            parts[2].replace("Course:", "").trim();

        const marks =
            parseFloat(
                parts[3].replace("Marks:", "").trim()
            );

        const attendance =
            parseFloat(
                parts[4]
                    .replace("Attendance:", "")
                    .replace("%", "")
                    .trim()
            );

        studentRecords.push({
            id,
            name,
            course,
            marks,
            attendance
        });

    });

}


/* =========================================================
   DISPLAY STUDENTS
========================================================= */

function displayStudents(result) {

    const tableBody =
        document.getElementById("studentTable");

    if (!tableBody)
        return;

    tableBody.innerHTML = "";

    const lines = result.split("\n");

    let count = 0;

    lines.forEach(function (line) {

        line = line.trim();

        if (!line.startsWith("ID:"))
            return;

        const parts = line.split("|");

        if (parts.length < 5)
            return;

        const id =
            parts[0].replace("ID:", "").trim();

        const name =
            parts[1].replace("Name:", "").trim();

        const course =
            parts[2].replace("Course:", "").trim();

        const marks =
            parts[3].replace("Marks:", "").trim();

        const attendance =
            parts[4]
                .replace("Attendance:", "")
                .replace("%", "")
                .trim();

        const row =
            document.createElement("tr");

        row.innerHTML = `
            <td>${id}</td>
            <td>${name}</td>
            <td>${course}</td>
            <td>${marks}</td>
            <td>${attendance}%</td>
        `;

        tableBody.appendChild(row);

        count++;

    });

    const totalStudents =
        document.getElementById("totalStudents");

    if (totalStudents) {
        totalStudents.textContent = count;
    }

    if (count === 0) {

        tableBody.innerHTML = `
            <tr>
                <td colspan="5" class="text-center">
                    No student records found
                </td>
            </tr>
        `;

    }

}


/* =========================================================
   ANALYTICS
========================================================= */

function updateAnalytics() {

    if (studentRecords.length === 0)
        return;

    let totalMarks = 0;
    let totalAttendance = 0;

    let highestMarks = 0;
    let alertCount = 0;

    studentRecords.forEach(function (student) {

        totalMarks += student.marks;

        totalAttendance += student.attendance;

        if (student.marks > highestMarks) {
            highestMarks = student.marks;
        }

        if (student.attendance < 75) {
            alertCount++;
        }

    });

    const averageMarks =
        totalMarks / studentRecords.length;

    const averageAttendance =
        totalAttendance / studentRecords.length;

    const averageMarksElement =
        document.getElementById("averageMarks");

    const averageAttendanceElement =
        document.getElementById("averageAttendance");

    const highestMarksElement =
        document.getElementById("highestMarks");

    const attendanceAlertsElement =
        document.getElementById("attendanceAlerts");

    if (averageMarksElement) {
        averageMarksElement.textContent =
            averageMarks.toFixed(1);
    }

    if (averageAttendanceElement) {
        averageAttendanceElement.textContent =
            averageAttendance.toFixed(1) + "%";
    }

    if (highestMarksElement) {
        highestMarksElement.textContent =
            highestMarks.toFixed(1);
    }

    if (attendanceAlertsElement) {
        attendanceAlertsElement.textContent =
            alertCount;
    }

    updateAttendanceAlerts();

}


/* =========================================================
   ATTENDANCE ALERTS
========================================================= */

function updateAttendanceAlerts() {

    const container =
        document.getElementById("attendanceAlertList");

    if (!container)
        return;

    const lowAttendance =
        studentRecords.filter(
            student => student.attendance < 75
        );

    if (lowAttendance.length === 0) {

        container.innerHTML = `
            <div class="empty-alert">
                <i class="bi bi-check-circle-fill"></i>
                <span>No attendance alerts</span>
            </div>
        `;

        return;
    }

    container.innerHTML = "";

    lowAttendance.forEach(function (student) {

        const item =
            document.createElement("div");

        item.className =
            "attendance-alert";

        item.innerHTML = `
            <div class="attendance-alert-info">
                <strong>${student.name}</strong>
                <span>ID: ${student.id}</span>
            </div>

            <div class="attendance-percent">
                ${student.attendance}%
            </div>
        `;

        container.appendChild(item);

    });

}


/* =========================================================
   ADD STUDENT
========================================================= */

const insertForm =
    document.getElementById("insertForm");

if (insertForm) {

    insertForm.addEventListener(
        "submit",
        async function (event) {

            event.preventDefault();

            const id =
                document.getElementById("studentId")
                    .value.trim();

            const name =
                document.getElementById("studentName")
                    .value.trim();

            const course =
                document.getElementById("studentCourse")
                    .value.trim();

            const marks =
                document.getElementById("studentMarks")
                    .value.trim();

            const attendance =
                document.getElementById("studentAttendance")
                    .value.trim();

            const command =
                `INSERT ${id} ${name} ${course} ${marks} ${attendance}`;

            const result =
                await sendCommand(command);

            alert(result);

            if (result.includes("SUCCESS")) {

                addActivity(
                    "Student inserted",
                    `${name} added to database`,
                    "green"
                );

            }

            insertForm.reset();

            await displayRecords();

        }
    );

}


/* =========================================================
   SEARCH
========================================================= */

const searchForm =
    document.getElementById("searchForm");

if (searchForm) {

    searchForm.addEventListener(
        "submit",
        async function (event) {

            event.preventDefault();

            const id =
                document.getElementById("searchId")
                    .value.trim();

            const searchResult =
                document.getElementById("searchResult");

            if (!id) {

                searchResult.innerHTML = `
                    <div class="search-error">
                        Please enter a Student ID.
                    </div>
                `;

                return;
            }

            searchResult.innerHTML = `
                <div>
                    Searching for Student ID
                    <strong>${id}</strong>...
                </div>
            `;

            const result =
                await sendCommand(`SEARCH ${id}`);

            console.log(
                "SEARCH RESPONSE:",
                result
            );

            if (result.startsWith("SUCCESS:")) {

                const match =
                    result.match(
                        /ID=(-?\d+)\s+Name=(.*?)\s+Course=(.*?)\s+Marks=([\d.]+)\s+Attendance=([\d.]+)/
                    );

                if (match) {

                    searchResult.innerHTML = `
                        <div class="search-success">

                            <h5>Student Found</h5>

                            <p>
                                <strong>ID:</strong>
                                ${match[1]}
                            </p>

                            <p>
                                <strong>Name:</strong>
                                ${match[2]}
                            </p>

                            <p>
                                <strong>Course:</strong>
                                ${match[3]}
                            </p>

                            <p>
                                <strong>Marks:</strong>
                                ${match[4]}
                            </p>

                            <p>
                                <strong>Attendance:</strong>
                                ${match[5]}%
                            </p>

                        </div>
                    `;

                    addActivity(
                        "Student searched",
                        `Student ID ${id}`,
                        "blue"
                    );

                } else {

                    searchResult.textContent = result;

                }

            } else {

                searchResult.innerHTML = `
                    <div class="search-error">

                        <strong>
                            Student not found.
                        </strong>

                        <br><br>

                        No record exists for Student ID
                        <strong>${id}</strong>.

                    </div>
                `;

            }

        }
    );

}


/* =========================================================
   UPDATE
========================================================= */

const updateForm =
    document.getElementById("updateForm");

if (updateForm) {

    updateForm.addEventListener(
        "submit",
        async function (event) {

            event.preventDefault();

            const id =
                document.getElementById("updateId")
                    .value.trim();

            const marks =
                document.getElementById("updateMarks")
                    .value.trim();

            const attendance =
                document.getElementById("updateAttendance")
                    .value.trim();

            const result =
                await sendCommand(
                    `UPDATE ${id} ${marks} ${attendance}`
                );

            alert(result);

            if (result.includes("SUCCESS")) {

                addActivity(
                    "Student updated",
                    `Student ID ${id}`,
                    "orange"
                );

            }

            updateForm.reset();

            await displayRecords();

        }
    );

}


/* =========================================================
   DELETE
========================================================= */

const deleteForm =
    document.getElementById("deleteForm");

if (deleteForm) {

    deleteForm.addEventListener(
        "submit",
        async function (event) {

            event.preventDefault();

            const id =
                document.getElementById("deleteId")
                    .value.trim();

            const result =
                await sendCommand(
                    `DELETE ${id}`
                );

            alert(result);

            if (result.includes("SUCCESS")) {

                addActivity(
                    "Student deleted",
                    `Student ID ${id}`,
                    "red"
                );

            }

            deleteForm.reset();

            await displayRecords();

        }
    );

}


/* =========================================================
   SYSTEM MONITORING
========================================================= */

async function loadSystemHealth() {

    const result =
        await sendCommand("MONITOR");

    console.log(
        "Monitor:",
        result
    );

    const activeMatch =
        result.match(
            /Active Clients:\s*(\d+)/i
        );

    const workerMatch =
        result.match(
            /Worker Threads:\s*(\d+)/i
        );

    const threadMatch =
        result.match(
            /Total Threads:\s*(\d+)/i
        );

    const memoryMatch =
        result.match(
            /Memory Usage:\s*(\d+)\s*kB/i
        );


    /* ACTIVE CLIENTS */

    if (activeMatch) {

        const activeElement =
            document.getElementById("activeClients");

        if (activeElement) {
            activeElement.textContent =
                activeMatch[1];
        }

    }


    /* WORKER THREADS */

    if (workerMatch) {

        const workerElement =
            document.getElementById("workerThreads");

        const healthWorkerElement =
            document.getElementById("healthWorkers");

        if (workerElement) {
            workerElement.textContent =
                workerMatch[1];
        }

        if (healthWorkerElement) {
            healthWorkerElement.textContent =
                workerMatch[1];
        }

    }


    /* TOTAL THREADS */

    if (threadMatch) {

        const threadElement =
            document.getElementById("totalThreads");

        const healthThreadElement =
            document.getElementById("healthThreads");

        if (threadElement) {
            threadElement.textContent =
                threadMatch[1];
        }

        if (healthThreadElement) {
            healthThreadElement.textContent =
                threadMatch[1];
        }

    }


    /* MEMORY USAGE */

    if (memoryMatch) {

        const memory =
            memoryMatch[1] + " kB";

        const healthMemoryElement =
            document.getElementById("healthMemory");

        if (healthMemoryElement) {
            healthMemoryElement.textContent =
                memory;
        }

    }

}


/* =========================================================
   RECENT ACTIVITY
========================================================= */

function addActivity(title, description, type) {

    recentActivities.unshift({

        title: title,

        description: description,

        type: type,

        time: "Just now"

    });

    if (recentActivities.length > 5) {

        recentActivities =
            recentActivities.slice(0, 5);

    }

    renderActivities();

}


function renderActivities() {

    const container =
        document.getElementById("activityList");

    if (!container)
        return;

    container.innerHTML = "";

    recentActivities.forEach(function (activity) {

        const item =
            document.createElement("div");

        item.className =
            "activity-item";

        item.innerHTML = `
            <div class="activity-dot ${activity.type}-dot"></div>

            <div>
                <strong>
                    ${activity.title}
                </strong>

                <span>
                    ${activity.description}
                </span>
            </div>

            <small>
                ${activity.time}
            </small>
        `;

        container.appendChild(item);

    });

}


/* =========================================================
   PERIODIC MONITORING
========================================================= */

setInterval(function () {

    if (currentUsername !== "") {

        loadSystemHealth();

    }

}, 3000);


/* =========================================================
   INITIAL ACTIVITY
========================================================= */

addActivity(
    "Dashboard initialized",
    "System ready",
    "blue"
);


console.log(
    "Dashboard JavaScript Ready"
);
