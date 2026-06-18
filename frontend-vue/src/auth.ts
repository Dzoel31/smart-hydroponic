import { reactive } from "vue";

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

const storedUser = getStoredUser();

export const authState = reactive({
    isLoggedIn: !!storedUser,
    user: storedUser,

    setSession(user: unknown) {
        localStorage.setItem('user', JSON.stringify(user));

        this.isLoggedIn = true;
        this.user = user;
    },

    logout() {
        localStorage.removeItem('user');

        this.isLoggedIn = false;
        this.user = null;
    }
});