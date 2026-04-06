import { reactive } from "vue";
import { OpenAPI } from "./api";

const getStoredUser = () => {
    const storedUser = localStorage.getItem('user');
    if (!storedUser) return null;

    try {
        return JSON.parse(storedUser);
    } catch {
        localStorage.removeItem('user');
        return null;
    }
};

const storedToken = localStorage.getItem('token');
if (storedToken) {
    OpenAPI.TOKEN = storedToken;
}

export const authState = reactive({
    isLoggedIn: !!storedToken,
    user: getStoredUser(),

    setSession(token: string, user: unknown) {
        localStorage.setItem('token', token);
        localStorage.setItem('user', JSON.stringify(user));
        OpenAPI.TOKEN = token;

        this.isLoggedIn = true;
        this.user = user;
    },

    logout() {
        localStorage.removeItem('token');
        localStorage.removeItem('user');
        OpenAPI.TOKEN = undefined;

        this.isLoggedIn = false;
        this.user = null;
    }
});